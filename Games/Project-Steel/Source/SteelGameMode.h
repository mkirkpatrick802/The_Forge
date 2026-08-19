#pragma once
#include <unordered_map>

#include "SteelTypes.h"
#include "Engine/GameModeBase.h"

// The rules of Project Steel: who is on which side, and what phase the match is in.
//
// Authority-only, like every game mode -- a level that arrives over the wire is built
// without one, so none of this exists on a client. Clients learn the results through
// the events in SteelTypes.h and through replicated TeamMember components.
class SteelGameMode final : public Engine::GameModeBase
{
public:
    SteelGameMode();
    ~SteelGameMode() override;

    void Start() override;
    void Update(float deltaTime) override;

    Engine::GameObject* SpawnPlayer(uint64_t playerID) override;

    void OnPlayerIdentified(NetCode::PeerID peer, const NetCode::PlayerIdentity& identity) override;
    void OnPlayerLeft(NetCode::PeerID peer, const NetCode::PlayerIdentity& identity) override;

private:
    // --- teams ---
    // Honours the player's preference unless it would make the sides uneven, in which
    // case they are placed on the other side as a mercenary. Returns what they got and
    // whether it was what they wanted.
    ETeam AssignTeam(uint64_t playerID, bool& outIsMercenary) const;

    void ApplyTeam(NetCode::PeerID peer, ETeam team, bool isMercenary);
    int CountTeam(ETeam team) const;

    // --- match flow ---
    void EnterState(EMatchState state);
    void BroadcastMatchState() const;
    void ResetMap();

    // --- requests ---
    void HandlePlaceShipPiece(NetCode::PeerID from, NetCode::InputByteStream& payload);
    void HandleSetTeamPreference(NetCode::PeerID from, NetCode::InputByteStream& payload);

private:
    // What each player asked for, by player id rather than peer id, so it survives a
    // reconnect within one server session. Persisting it across restarts is what the
    // identity work is for and is not wired up yet.
    std::unordered_map<uint64_t, ETeam> _preferredTeam;

    // The side each connected peer actually ended up on.
    std::unordered_map<NetCode::PeerID, ETeam> _assignedTeam;

    EMatchState _state = EMatchState::Lobby;
    float _stateTimer = 0.0f;

    // An admin said go. The population check that cancels a countdown is there to stop
    // a match starting because someone wandered in and out -- it must not overrule a
    // human who deliberately started one.
    bool _startForced = false;

    // Enough players present and the match starts itself after the countdown. An
    // admin can also force it with the `startmatch` console command.
    static constexpr int PLAYERS_TO_START = 4;
    static constexpr float COUNTDOWN_SECONDS = 30.0f;
    static constexpr float POST_MATCH_SECONDS = 15.0f;
};

REGISTER_GAME_MODE(SteelGameMode)
