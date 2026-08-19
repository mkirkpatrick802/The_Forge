#include "SteelGameMode.h"

#include "ShipPiece.h"
#include "TeamMember.h"
#include "Engine/CommandRegistry.h"
#include "Engine/GameObject.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"
#include "Engine/Components/PlayerController.h"
#include "NetworkManager.h"

using namespace Engine;
using namespace NetCode;

SteelGameMode::SteelGameMode()
{
    _playerPrefab = "Assets/Prefabs/Player.prefab";

    // Registered here rather than at static-init time because these handlers close
    // over this mode: they are the rules, and the rules are what a game mode is.
    NetActionRegistry::RegisterRequest(SteelRequest::PlaceShipPiece,
        [this](const PeerID from, InputByteStream& payload) { HandlePlaceShipPiece(from, payload); });

    NetActionRegistry::RegisterRequest(SteelRequest::SetTeamPreference,
        [this](const PeerID from, InputByteStream& payload) { HandleSetTeamPreference(from, payload); });

    // The concept asks for a way to start the match by hand. On a headless server this
    // is reachable through the console reader; in the editor it is reachable from chat.
    CommandRegistry::RegisterCommand("startmatch", [this](const std::string&)
    {
        if (_state == EMatchState::Lobby || _state == EMatchState::PostMatch)
        {
            DEBUG_LOG("Match: starting by admin command.")
            _startForced = true;
            EnterState(EMatchState::Countdown);
        }
        else
        {
            DEBUG_LOG("Match: already %s.", MatchStateName(_state))
        }
    });

    CommandRegistry::RegisterCommand("teams", [this](const std::string&)
    {
        DEBUG_LOG("Match: %s -- Red %d, Blue %d.", MatchStateName(_state), CountTeam(ETeam::Red), CountTeam(ETeam::Blue))
    });
}

SteelGameMode::~SteelGameMode()
{
    NetActionRegistry::UnregisterRequest(SteelRequest::PlaceShipPiece);
    NetActionRegistry::UnregisterRequest(SteelRequest::SetTeamPreference);
    CommandRegistry::UnregisterCommand("startmatch");
    CommandRegistry::UnregisterCommand("teams");
}

void SteelGameMode::Start()
{
    GameModeBase::Start();
    DEBUG_LOG("Match: %s. Waiting for %d players.", MatchStateName(_state), PLAYERS_TO_START)
}

/*
 *      Match flow
 */
void SteelGameMode::Update(const float deltaTime)
{
    switch (_state)
    {
    case EMatchState::Lobby:
        // The server never stops running, so the lobby is playable rather than a
        // waiting room -- people mine and build in it. All that is missing is that a
        // result does not count.
        if (static_cast<int>(GetNetworkManager().GetConnectedPlayers().size()) >= PLAYERS_TO_START)
            EnterState(EMatchState::Countdown);
        break;

    case EMatchState::Countdown:
        _stateTimer -= deltaTime;

        // Dropping back below the threshold cancels rather than starting a match one
        // player will be alone in.
        if (!_startForced && static_cast<int>(GetNetworkManager().GetConnectedPlayers().size()) < PLAYERS_TO_START)
        {
            DEBUG_LOG("Match: not enough players, countdown cancelled.")
            EnterState(EMatchState::Lobby);
        }
        else if (_stateTimer <= 0.0f)
        {
            EnterState(EMatchState::Live);
        }
        break;

    case EMatchState::Live:
        // Win conditions land here once bases exist. Until then a live match simply
        // runs; nothing can end it.
        break;

    case EMatchState::PostMatch:
        _stateTimer -= deltaTime;
        if (_stateTimer <= 0.0f)
            EnterState(EMatchState::Lobby);
        break;
    }
}

void SteelGameMode::EnterState(const EMatchState state)
{
    _state = state;

    switch (state)
    {
    case EMatchState::Countdown: _stateTimer = COUNTDOWN_SECONDS; break;
    case EMatchState::PostMatch: _stateTimer = POST_MATCH_SECONDS; break;
    default:                     _stateTimer = 0.0f; break;
    }

    // Consumed by whatever the countdown becomes, so a forced start does not linger
    // and suppress the population check on the next one.
    if (state != EMatchState::Countdown)
        _startForced = false;

    if (state == EMatchState::Live)
        ResetMap();

    DEBUG_LOG("Match: -> %s (Red %d, Blue %d).", MatchStateName(state), CountTeam(ETeam::Red), CountTeam(ETeam::Blue))
    BroadcastMatchState();
}

void SteelGameMode::BroadcastMatchState() const
{
    OutputByteStream payload;
    payload.Write(static_cast<uint8_t>(_state));
    payload.Write(_stateTimer);
    GetNetworkManager().BroadcastEvent(SteelEvent::MatchStateChanged, payload);
}

void SteelGameMode::ResetMap()
{
    // Placeholder with a deliberate shape: a match starting is the point at which the
    // world goes back to its authored state and teams are rebalanced. Doing it properly
    // needs a level reload that keeps connections alive -- every peer's replication
    // slot has to be reset and re-snapshotted, since every network id it knows about
    // becomes invalid. That is the next piece of work here.
    DEBUG_LOG("Match: map reset requested -- not yet implemented.")
}

/*
 *      Teams
 */
int SteelGameMode::CountTeam(const ETeam team) const
{
    int count = 0;
    for (const auto& [peer, assigned] : _assignedTeam)
        if (assigned == team)
            ++count;

    return count;
}

ETeam SteelGameMode::AssignTeam(const uint64_t playerID, bool& outIsMercenary) const
{
    const auto preference = _preferredTeam.find(playerID);
    const ETeam wanted = preference != _preferredTeam.end() ? preference->second : ETeam::None;

    const int red = CountTeam(ETeam::Red);
    const int blue = CountTeam(ETeam::Blue);

    // No preference: simply even the sides, tie-breaking to Red.
    if (wanted == ETeam::None)
    {
        outIsMercenary = false;
        return red <= blue ? ETeam::Red : ETeam::Blue;
    }

    // Preference honoured unless taking it would put this side two ahead -- one ahead
    // is the unavoidable state of an odd number of players, and bouncing people off
    // their chosen faction for that would make the preference nearly meaningless.
    const int wantedCount = wanted == ETeam::Red ? red : blue;
    const int otherCount = wanted == ETeam::Red ? blue : red;

    if (wantedCount > otherCount)
    {
        outIsMercenary = true;
        return OtherTeam(wanted);
    }

    outIsMercenary = false;
    return wanted;
}

void SteelGameMode::ApplyTeam(const PeerID peer, const ETeam team, const bool isMercenary)
{
    _assignedTeam[peer] = team;

    if (const auto controller = GetNetworkManager().FindPlayerController(peer))
    {
        // Added at spawn rather than authored into the prefab, so the rules decide the
        // side rather than the asset. Replicates as part of the pawn like any component.
        auto member = controller->gameObject->GetComponent<TeamMember>();
        if (member == nullptr)
            member = controller->gameObject->AddComponent<TeamMember>();

        member->SetTeam(team);
    }

    OutputByteStream payload;
    payload.Write(static_cast<uint8_t>(team));
    payload.Write(isMercenary);
    GetNetworkManager().SendEventTo(peer, SteelEvent::TeamAssigned, payload);
}

Engine::GameObject* SteelGameMode::SpawnPlayer(const uint64_t playerID)
{
    Engine::GameObject* pawn = GameModeBase::SpawnPlayer(playerID);

    // The team is not known here. A pawn is spawned the moment a peer connects, which
    // is before it has said who it is -- assignment happens in OnPlayerIdentified.
    if (pawn != nullptr && pawn->GetComponent<TeamMember>() == nullptr)
        pawn->AddComponent<TeamMember>();

    return pawn;
}

void SteelGameMode::OnPlayerIdentified(const PeerID peer, const PlayerIdentity& identity)
{
    bool isMercenary = false;
    const ETeam team = AssignTeam(identity.id, isMercenary);
    ApplyTeam(peer, team, isMercenary);

    DEBUG_LOG("Match: %s joined %s%s.", identity.name.c_str(), TeamName(team), isMercenary ? " (mercenary)" : "")

    // Bring the new arrival up to date on where the match is, which the world state
    // does not carry -- it is a property of the rules, not of any object.
    OutputByteStream payload;
    payload.Write(static_cast<uint8_t>(_state));
    payload.Write(_stateTimer);
    GetNetworkManager().SendEventTo(peer, SteelEvent::MatchStateChanged, payload);
}

void SteelGameMode::OnPlayerLeft(const PeerID peer, const PlayerIdentity& identity)
{
    _assignedTeam.erase(peer);
    DEBUG_LOG("Match: %s left. Red %d, Blue %d.", identity.name.c_str(), CountTeam(ETeam::Red), CountTeam(ETeam::Blue))
}

/*
 *      Requests
 */
void SteelGameMode::HandleSetTeamPreference(const PeerID from, InputByteStream& payload)
{
    uint8_t raw;
    payload.Read(raw);

    if (raw > static_cast<uint8_t>(ETeam::Blue))
    {
        GetNetworkManager().DenyRequest(from, "unknown team");
        return;
    }

    const PlayerIdentity* identity = GetNetworkManager().FindIdentity(from);
    if (identity == nullptr)
    {
        GetNetworkManager().DenyRequest(from, "not joined yet");
        return;
    }

    _preferredTeam[identity->id] = static_cast<ETeam>(raw);

    // Taking effect on the next assignment rather than immediately: switching sides
    // mid-match by asking is a different feature, and one with balance consequences.
    DEBUG_LOG("Match: %s prefers %s from next assignment.", identity->name.c_str(), TeamName(static_cast<ETeam>(raw)))
}

void SteelGameMode::HandlePlaceShipPiece(const PeerID from, InputByteStream& payload)
{
    uint8_t rawType;
    glm::vec2 position;
    float rotation;
    uint32_t attachToNetworkID;

    payload.Read(rawType);
    payload.Read(position);
    payload.Read(rotation);
    payload.Read(attachToNetworkID);

    // Everything above came from a client and none of it is trusted. The acting player
    // is resolved from `from`, which the transport supplied, and never from the payload.
    const PlayerIdentity* identity = GetNetworkManager().FindIdentity(from);
    if (identity == nullptr)
    {
        GetNetworkManager().DenyRequest(from, "not joined yet");
        return;
    }

    if (rawType >= static_cast<uint8_t>(EShipPieceType::Max))
    {
        GetNetworkManager().DenyRequest(from, "unknown piece type");
        return;
    }

    const auto controller = GetNetworkManager().FindPlayerController(from);
    if (controller == nullptr)
    {
        GetNetworkManager().DenyRequest(from, "no pawn to build from");
        return;
    }

    if (!ShipPiece::PlaceAuthoritative(static_cast<EShipPieceType>(rawType), position, rotation,
                                       attachToNetworkID, controller->gameObject))
    {
        GetNetworkManager().DenyRequest(from, "cannot build there");
    }
}
