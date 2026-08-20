#include "NetworkManager.h"

#include <memory>

#include "LinkingContext.h"
#include "GamerServices.h"
#include "NetActions.h"
#include "SteamGameServerAuth.h"
#include "Transport/OfflineTransport.h"
#include "Transport/SteamTransport.h"
#include "Transport/UdpNetTransport.h"
#include "Engine/GameModeBase.h"
#include "Engine/LaunchOptions.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"
#include "Engine/Time.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/PlayerController.h"

NetCode::NetworkManager& NetCode::NetworkManager::GetInstance()
{
    static auto instance = std::make_unique<NetworkManager>();
    return *instance;
}

NetCode::NetworkManager::NetworkManager()
{
    // Deliberately no transport yet. It used to be chosen here, which was correct while
    // the role could only come from the command line -- but a main menu picks the role
    // and the server address at run time, and this singleton is constructed long before
    // the player has pressed anything. Choosing at construction meant a client that
    // booted into a menu had already committed to a transport by the time it was asked
    // which server to join.
    //
    // Until StartNetCode runs, _transport is null and Update is a no-op.
    RegisterEngineActions();
}

void NetCode::NetworkManager::SelectTransport()
{
    // One decision, made once per session: which transport this process is going to use.
    // Every "am I a server", "is Steam up", "who am I" question downstream resolves
    // through the object chosen here rather than being re-derived from the role.
    const Engine::LaunchOptions& options = Engine::GetLaunchOptions();

    // A previous session's transport is shut down and dropped first. Reusing one that
    // has already been through Shutdown is how a second connect in one session ends up
    // silently talking to a closed socket.
    if (_transport != nullptr)
        _transport->Shutdown();

    if (options.UsesDedicatedServerModel())
        _transport = std::make_unique<UdpNetTransport>();
    else if (Engine::REQUIRE_GAMER_SERVICES)
        _transport = std::make_unique<SteamTransport>();
    else
        _transport = std::make_unique<OfflineTransport>();
}

void NetCode::NetworkManager::RegisterEngineActions()
{
    NetActionRegistry::RegisterRequest(EngineRequest::Join,
        [this](const PeerID from, InputByteStream& payload) { HandleJoinRequest(from, payload); });

    NetActionRegistry::RegisterEvent(EngineEvent::JoinAccepted,
        [this](InputByteStream&) { _hasJoined = true; DEBUG_LOG("Netcode: the server accepted our identity.") });

    NetActionRegistry::RegisterEvent(EngineEvent::RequestDenied,
        [](InputByteStream& payload)
        {
            std::string reason;
            payload.Read(reason);
            DEBUG_LOG("Netcode: the server refused a request -- %s", reason.c_str())
        });
}

NetCode::NetworkManager::~NetworkManager()
{
    ShutdownNetCode();
}

void NetCode::NetworkManager::ShutdownNetCode()
{
    // Guarded because main() shuts down explicitly -- so the goodbye packets go out
    // while the world is still standing -- and the destructor still has to cover
    // any path that does not get that far.
    if (_shutdown) return;
    _shutdown = true;

    // Released before the transport goes, so Steam is told about the departure rather
    // than inferring it from a server that stopped answering.
    if (Engine::REQUIRE_GAMER_SERVICES && GamerServices::instance != nullptr)
        GetGamerService().CancelAuthTicket();

    SteamGameServerAuth::Get().Shutdown();

    // Null before the first StartNetCode, which is the state a process that never got
    // past the main menu exits in.
    if (_transport != nullptr)
        _transport->Shutdown();

    _state = NMS_Unitialized;
}

void NetCode::NetworkManager::StartNetCode()
{
    _state = NMS_Searching;

    // Cleared, because ShutdownNetCode latches it to make itself idempotent. Leaving it
    // set means a second session in the same process -- quit to the menu, then press
    // Play again -- would skip its own shutdown later and never say goodbye.
    _shutdown = false;

    // Per-session client state. A reconnect that inherited the last session's "we have
    // already joined" would never send its Join.
    _hasJoined = false;
    _joinSent = false;
    _awaitingTicket = false;
    _players.clear();
    _pendingAuth.clear();
    _pendingKicks.clear();

    const Engine::LaunchOptions& options = Engine::GetLaunchOptions();

    // Reads the role and address as they stand now, which is what lets a menu choose
    // them.
    SelectTransport();

    if (options.IsDedicatedServer())
    {
        _requireAuth = !options.insecure;

        if (_requireAuth)
        {
            SteamGameServerAuth::Get().onValidated =
                [this](const uint64_t steamID, const bool ok, const char* reason) { OnAuthValidated(steamID, ok, reason); };

            // Refusing to start is the point. Falling back to accepting unverified
            // players because Steam was missing would mean the security of a server
            // depended on whether a DLL happened to be installed -- and nobody would
            // notice until it mattered. --insecure is how you say you meant it.
            // A query port of its own rather than sharing the game port: sharing means
            // Steam does not open a socket and the game must forward server-browser
            // packets itself through HandleIncomingPacket, which this transport does not.
            if (!SteamGameServerAuth::Get().Start(options.port, static_cast<uint16_t>(options.port + 1), options.gsltToken))
            {
                DEBUG_LOG("Netcode: refusing to start. Steam authentication is unavailable and --insecure was not given.")
                APPLICATION_CLOSING = true;
                return;
            }

            DEBUG_LOG("Netcode: authenticating server -- players must prove their Steam identity.")
        }
        else
        {
            DEBUG_LOG("Netcode: *** INSECURE SERVER *** identities are taken at face value. Development and LAN only.")
        }
    }

    if (!_transport->Start())
    {
        DEBUG_LOG("Netcode: transport failed to start.")
        return;
    }

    // The authority is playing the moment its level is up -- it has nobody to wait
    // for. A client stays in NMS_Starting until the world state arrives.
    _state = HasWorldAuthority() ? NMS_Playing : NMS_Starting;

    // With no session there is nobody to announce us, so the local player would
    // never be spawned and the camera would have nothing to follow. This is what
    // makes an offline or editor-launched game come up with a player at all.
    // The authority always has a pawn of its own, unless it is a dedicated server --
    // that is the one authority with no local player, which is exactly why it ends up
    // with an empty world of its own.
    //
    // Deliberately *not* keyed on IsSessionActive(). It used to be, and that was wrong in
    // a way that only showed on the second play of a session: the transport is owned by
    // this singleton and outlives any level, so a Steam lobby joined during the first
    // play is still open when the level reloads. The second play therefore saw an active
    // session, skipped local onboarding entirely, and came up with no pawn, no camera and
    // a blank screen. Whether a session exists says nothing about whether *this* machine
    // already has a player in the level.
    const bool isDedicatedServer = Engine::GetLaunchOptions().IsDedicatedServer();

    if (HasWorldAuthority() && !isDedicatedServer)
    {
        // Idempotent: a level reload destroys the pawn, and StartNetCode runs again for
        // every level that starts, so the test is "is there a pawn" rather than "is this
        // the first time".
        if (FindPlayerController(_transport->GetLocalPeerID()) == nullptr)
            OnboardNewPlayer(_transport->GetLocalPeerID());
    }
}

void NetCode::NetworkManager::Update()
{
    // No session yet. The game loop calls this every frame from the moment it starts, so
    // while a client sits on the main menu -- before it has chosen a server -- there is
    // no transport to pump and dereferencing one would be the first thing that happened.
    if (_transport == nullptr || _state == NMS_Unitialized) return;

    // Pumped here rather than inside a transport, because identity is not a transport
    // concern: a client talking UDP to a dedicated server still needs Steam client
    // callbacks for its auth ticket, and used to never get them -- GetAuthSessionTicket
    // handed over a handle and the confirming callback simply never fired.
    if (Engine::REQUIRE_GAMER_SERVICES && GamerServices::instance != nullptr)
        GetGamerService().Update();

    _transport->Update();

    // Pumped before anything reads the results: the validation verdict for a peer that
    // joined last frame should land before this frame decides what to do about it.
    SteamGameServerAuth::Get().Update();

    ProcessConnectionEvents();
    ProcessIncomingPackets();
    UpdatePendingAuth();
    SendJoinWhenReady();
    SendWorldStateUpdate();
    SendClientInput();
}

void NetCode::NetworkManager::ProcessConnectionEvents()
{
    for (const PeerID peer : _transport->TakeConnectedPeers())
        WelcomePeer(peer);

    for (const PeerID peer : _transport->TakeDisconnectedPeers())
        HandlePeerDisconnected(peer);
}

Engine::PlayerController* NetCode::NetworkManager::FindPlayerController(const PeerID peer) const
{
    const std::vector<Engine::Component*> components = Engine::GetComponentManager().GetAllDerivedComponents<Engine::PlayerController>();
    for (const auto component : components)
    {
        if (const auto player = dynamic_cast<Engine::PlayerController*>(component))
        {
            if (player->GetControllingPlayerID() == peer)
                return player;
        }
    }

    return nullptr;
}

void NetCode::NetworkManager::ProcessIncomingPackets()
{
    // Whole messages, in order, already reassembled -- the transport does not hand
    // up anything partial, so there is no framing left to unpick here.
    for (NetMessage& message : _transport->Receive())
        ProcessPacket(message.stream, message.from);
}

void NetCode::NetworkManager::ProcessPacket(InputByteStream& stream, const PeerID peer)
{
    PacketType type;
    stream.Read(type);
    switch (type)
    {
    case PT_Hello:
        {
            DEBUG_LOG("Host has welcomed us; proving who we are.")

            // Asking Steam for a ticket is asynchronous, so the Join goes out from
            // SendJoinWhenReady once the ticket is usable rather than from here.
            if (Engine::REQUIRE_GAMER_SERVICES && GamerServices::instance != nullptr)
                GetGamerService().RequestAuthTicket();

            _awaitingTicket = true;
            _joinSent = false;
        }
        break;
    case PT_WorldState:
        Engine::LevelManager::LoadLevel(stream);
        _state = NMS_Playing;
        break;
    case PT_WorldStateUpdate:
        if (_state != NMS_Playing) return;
        Engine::LevelManager::GetCurrentLevel()->Read(stream);
        break;
    case PT_ClientInput:
        ApplyClientInput(stream, peer);
        break;
    case PT_Request:
        {
            // Direction is enforced before the payload is looked at. A request is a
            // petition to the authority; one arriving anywhere else is either a
            // confused build or someone probing, and neither deserves parsing.
            if (!HasWorldAuthority())
            {
                DEBUG_LOG("Netcode: ignoring a request received on a non-authority.")
                return;
            }

            NetActionID id;
            stream.Read(id);

            // `peer` comes from the transport, not the payload. Every handler resolves
            // the acting player from it, which is what stops a client acting as another.
            if (!NetActionRegistry::DispatchRequest(id, peer, stream))
                DEBUG_LOG("Netcode: no handler for request %u from peer %llu.", id, peer)
        }
        break;
    case PT_Event:
        {
            // The mirror of the above: only the authority states facts, so an event
            // arriving at the authority is not from anyone entitled to send it.
            if (HasWorldAuthority())
            {
                DEBUG_LOG("Netcode: ignoring an event received on the authority.")
                return;
            }

            NetActionID id;
            stream.Read(id);

            if (!NetActionRegistry::DispatchEvent(id, stream))
                DEBUG_LOG("Netcode: no handler for event %u.", id)
        }
        break;
    default:
        DEBUG_LOG("Unhandled Packet Received!")
    }
}

void NetCode::NetworkManager::SendWorldStateUpdate()
{
    // World state is authoritative -- only the machine simulating it broadcasts.
    if (!HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    const std::vector<PeerID> peers = _transport->GetRemotePeers();
    if (peers.empty()) return;

    const uint64_t currentTicks = Engine::Time::GetTicks();
    if (currentTicks - _lastUpdateSentTicks < _targetStateUpdateDelayMs) return;
    _lastUpdateSentTicks = currentTicks;

    Engine::Level* level = Engine::LevelManager::GetCurrentLevel();
    if (level == nullptr) return;

    // One pass over the dirty world, not one per peer: every object still owed to
    // anybody is serialized once here, and each peer's update below is a splice of
    // the blobs that peer is missing. At 32 players the difference is 32x the
    // serialization work -- and object serialization is the expensive half, being
    // string writes and virtual component dispatch.
    level->BuildDeltaBlobs();

    for (const PeerID peer : peers)
    {
        if (!level->HasPendingDelta(peer)) continue;

        OutputByteStream stream;
        stream.Write(PT_WorldStateUpdate);
        level->WriteDeltaFor(peer, stream);

        // The peer's dirty bits are retired only once the transport has actually
        // accepted the message. A refused send -- a full reliable window, which is
        // exactly what a backed-up client produces -- leaves them set, so the update
        // coalesces into the peer's next one instead of being silently lost.
        //
        // This is the whole point of tracking dirtiness per peer. While the flags were
        // global there was no safe response to a refusal: the state had already been
        // cleared for everyone, so the peer desynced with nothing able to detect it,
        // and deliberately skipping a slow peer did the same damage.
        if (_transport->SendTo(peer, stream, true))
            level->CommitDeltaFor(peer);

    }
}

void NetCode::NetworkManager::SendClientInput()
{
    // The authority simulates its own input directly -- only remote clients send it.
    if (HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    const uint64_t currentTicks = Engine::Time::GetTicks();
    if (currentTicks - _lastInputSentTicks < _targetInputSendDelayMs) return;
    _lastInputSentTicks = currentTicks;

    const auto localPlayer = FindPlayerController(_transport->GetLocalPeerID());
    if (localPlayer == nullptr) return;

    OutputByteStream stream;
    stream.Write(PT_ClientInput);
    localPlayer->WriteInput(stream);

    // Unreliable on purpose. Input is superseded every tick -- ReadInput just
    // overwrites the last values -- so a dropped packet costs one frame of stale
    // input, where reliable-ordered delivery costs a retransmit and stalls every
    // input behind it. Sent reliably at 60Hz it also exhausted the send window
    // outright once a second client joined.
    //
    // World state cannot do this: Write(stream, false) is a *delta* that clears
    // the dirty flags, so a lost update is lost for good. That is why the two go
    // out on different channels.
    _transport->SendToAuthority(stream, false);
}

bool NetCode::NetworkManager::SendRequest(const NetActionID id, const OutputByteStream& payload)
{
    if (HasWorldAuthority()) return false;

    OutputByteStream stream;
    stream.Write(PT_Request);
    stream.Write(id);
    stream.WriteBits(payload.GetBuffer(), payload.GetBitLength());

    return _transport->SendToAuthority(stream, true);
}

bool NetCode::NetworkManager::SendEventTo(const PeerID peer, const NetActionID id, const OutputByteStream& payload)
{
    if (!HasWorldAuthority()) return false;

    OutputByteStream stream;
    stream.Write(PT_Event);
    stream.Write(id);
    stream.WriteBits(payload.GetBuffer(), payload.GetBitLength());

    return _transport->SendTo(peer, stream, true);
}

void NetCode::NetworkManager::BroadcastEvent(const NetActionID id, const OutputByteStream& payload)
{
    if (!HasWorldAuthority()) return;

    for (const PeerID peer : _transport->GetRemotePeers())
        SendEventTo(peer, id, payload);
}

bool NetCode::NetworkManager::DispatchLocalRequest(const NetActionID id, const OutputByteStream& payload)
{
    if (!HasWorldAuthority()) return false;

    InputByteStream stream(payload.GetBuffer(), payload.GetByteLength());
    return NetActionRegistry::DispatchRequest(id, GetLocalUserID(), stream);
}

void NetCode::NetworkManager::DenyRequest(const PeerID peer, const std::string& reason)
{
    OutputByteStream payload;
    payload.Write(reason);
    SendEventTo(peer, EngineEvent::RequestDenied, payload);
}

const NetCode::PlayerIdentity* NetCode::NetworkManager::FindIdentity(const PeerID peer) const
{
    for (const ConnectedPlayer& player : _players)
        if (player.peer == peer)
            return &player.identity;

    return nullptr;
}

NetCode::PeerID NetCode::NetworkManager::FindPeerByPlayerID(const uint64_t playerID) const
{
    for (const ConnectedPlayer& player : _players)
        if (player.identity.id == playerID)
            return player.peer;

    return INVALID_PEER;
}

NetCode::PendingJoin* NetCode::NetworkManager::FindPendingJoin(const PeerID peer)
{
    for (PendingJoin& pending : _pendingAuth)
        if (pending.peer == peer)
            return &pending;

    return nullptr;
}

void NetCode::NetworkManager::ErasePendingJoin(const PeerID peer)
{
    std::erase_if(_pendingAuth, [peer](const PendingJoin& pending) { return pending.peer == peer; });
}

void NetCode::NetworkManager::HandleJoinRequest(const PeerID from, InputByteStream& payload)
{
    PlayerIdentity identity;
    identity.Read(payload);

    if (!identity.IsValid())
    {
        KickPeer(from, "invalid player identity");
        return;
    }

    // Sent once. A second Join from the same peer would let a player change who they
    // are mid-session, which is the whole point of pinning identity to a connection.
    if (FindIdentity(from) != nullptr)
    {
        DenyRequest(from, "already joined");
        return;
    }

    PendingJoin* pending = FindPendingJoin(from);
    if (pending == nullptr)
    {
        // No open window: either it expired, or this peer was never welcomed.
        DenyRequest(from, "not expecting a join");
        return;
    }

    if (pending->identity.IsValid())
    {
        DenyRequest(from, "already authenticating");
        return;
    }

    // One id, one connection. Without this a second connection could claim an id that
    // is already playing and, once verified, collide with them in every map keyed on it.
    if (FindPeerByPlayerID(identity.id) != INVALID_PEER)
    {
        KickPeer(from, "that account is already on this server");
        return;
    }

    for (const PendingJoin& other : _pendingAuth)
    {
        if (other.peer == from) continue;
        if (other.identity.id == identity.id)
        {
            KickPeer(from, "that account is already authenticating");
            return;
        }
    }

    if (!_requireAuth)
    {
        // Insecure server: the claim is taken at its word. This is the branch that makes
        // development and LAN play possible and the branch that must never run on a
        // public server, which is why --insecure has to be typed.
        ErasePendingJoin(from);
        AdmitPlayer(from, identity);
        return;
    }

    if (identity.authTicket.empty())
    {
        KickPeer(from, "this server requires Steam authentication");
        return;
    }

    if (!SteamGameServerAuth::Get().BeginAuth(identity.id, identity.authTicket))
    {
        KickPeer(from, "Steam refused your auth ticket");
        return;
    }

    // Connected, not admitted: no pawn, no world state, no replication slot until Steam
    // says this really is who they say they are. The deadline is refreshed because what
    // is being waited on has changed -- from "answer us" to "Steam answers us".
    pending->identity = identity;
    pending->deadlineTicks = Engine::Time::GetTicks() + AUTH_TIMEOUT_MS;

    DEBUG_LOG("Netcode: peer %llu claims to be %s (%llu); waiting on Steam.", from, identity.name.c_str(), identity.id)
}

void NetCode::NetworkManager::OnAuthValidated(const uint64_t steamID, const bool ok, const char* reason)
{
    for (auto it = _pendingAuth.begin(); it != _pendingAuth.end(); ++it)
    {
        if (!it->identity.IsValid() || it->identity.id != steamID) continue;

        const PendingJoin pending = *it;
        _pendingAuth.erase(it);

        if (!ok)
        {
            // The session was begun, so it has to be ended even though it failed.
            SteamGameServerAuth::Get().EndAuth(steamID);
            KickPeer(pending.peer, reason != nullptr ? reason : "Steam rejected your identity");
            return;
        }

        AdmitPlayer(pending.peer, pending.identity);
        return;
    }

    // A verdict for somebody who already left. The session still has to be ended, or
    // Steam goes on believing they are playing here.
    SteamGameServerAuth::Get().EndAuth(steamID);
}

void NetCode::NetworkManager::UpdatePendingAuth()
{
    const uint64_t now = Engine::Time::GetTicks();

    for (auto it = _pendingKicks.begin(); it != _pendingKicks.end();)
    {
        if (now < it->atTicks) { ++it; continue; }

        _transport->Disconnect(it->peer);
        it = _pendingKicks.erase(it);
    }

    if (_pendingAuth.empty()) return;

    for (auto it = _pendingAuth.begin(); it != _pendingAuth.end();)
    {
        if (now < it->deadlineTicks)
        {
            ++it;
            continue;
        }

        // Steam never answered. Holding the connection open indefinitely would let
        // anyone occupy a slot by connecting and saying nothing further.
        const PendingJoin expired = *it;
        it = _pendingAuth.erase(it);

        if (expired.identity.IsValid())
        {
            SteamGameServerAuth::Get().EndAuth(expired.identity.id);
            KickPeer(expired.peer, "authentication timed out");
        }
        else
        {
            KickPeer(expired.peer, "did not identify in time");
        }
    }
}

void NetCode::NetworkManager::AdmitPlayer(const PeerID peer, const PlayerIdentity& identity)
{
    if (!_transport->HasPeer(peer) && peer != _transport->GetLocalPeerID())
    {
        DEBUG_LOG("Netcode: %s was verified but has already left.", identity.name.c_str())
        return;
    }

    _players.push_back(ConnectedPlayer{peer, identity});
    DEBUG_LOG("Netcode: peer %llu is %s (%llu).", peer, identity.name.c_str(), identity.id)

    SendEventTo(peer, EngineEvent::JoinAccepted, OutputByteStream());

    // Only now does the player get a body and a copy of the world.
    OnboardNewPlayer(peer);

    if (Engine::Level* level = Engine::LevelManager::GetCurrentLevel())
        level->GetGameMode().OnPlayerIdentified(peer, identity);
}

void NetCode::NetworkManager::KickPeer(const PeerID peer, const std::string& reason)
{
    DEBUG_LOG("Netcode: kicking peer %llu -- %s.", peer, reason.c_str())

    for (const PendingKick& kick : _pendingKicks)
        if (kick.peer == peer)
            return;

    // Told first, dropped a moment later. The delay is what lets the explanation
    // actually arrive -- see KICK_GRACE_MS.
    DenyRequest(peer, reason);
    _pendingKicks.push_back(PendingKick{peer, Engine::Time::GetTicks() + KICK_GRACE_MS});
}

void NetCode::NetworkManager::SendJoinWhenReady()
{
    if (!_awaitingTicket || _joinSent) return;
    if (HasWorldAuthority()) return;

    const bool steamUp = Engine::REQUIRE_GAMER_SERVICES && GamerServices::instance != nullptr;

    // Without Steam there is no ticket to wait for, and the server will either accept
    // the bare claim (--insecure) or refuse it. Either way, sending immediately is the
    // right move -- waiting would just stall until the auth window expired.
    if (steamUp && !GetGamerService().IsAuthTicketReady())
        return;

    OutputByteStream join;
    PlayerIdentity::Local().Write(join);

    if (!SendRequest(EngineRequest::Join, join)) return;

    _joinSent = true;
    _awaitingTicket = false;
}

void NetCode::NetworkManager::ApplyClientInput(InputByteStream& stream, const PeerID peer)
{
    // Only the authority consumes client input.
    if (!HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    // The pawn is resolved from the *sender's* peer id rather than anything in the
    // payload, so a client can only ever drive the player it actually controls.
    if (const auto player = FindPlayerController(peer))
        player->ReadInput(stream);
}

// Says hello, and nothing else. A peer that has just connected has not proved anything
// yet, so it gets no pawn, no world state and no replication slot -- only the chance to
// send its identity. AdmitPlayer is where those things happen.
void NetCode::NetworkManager::WelcomePeer(const PeerID peer)
{
    if (!HasWorldAuthority()) return;

    if (Engine::LevelManager::GetCurrentLevel() == nullptr)
    {
        DEBUG_LOG("Netcode: peer %llu arrived before a level existed; ignoring.", peer)
        return;
    }

    // The local peer is not a remote arrival and has nothing to prove to itself.
    if (peer == _transport->GetLocalPeerID())
    {
        // Identity is recorded once per session; the pawn is spawned whenever there is
        // not one. Tying the spawn to the identity meant that after the first level the
        // local player was never given a body again.
        if (FindIdentity(peer) == nullptr)
            _players.push_back(ConnectedPlayer{peer, PlayerIdentity::Local()});

        if (FindPlayerController(peer) == nullptr)
        {
            OnboardNewPlayer(peer);

            if (const PlayerIdentity* identity = FindIdentity(peer))
                Engine::LevelManager::GetCurrentLevel()->GetGameMode().OnPlayerIdentified(peer, *identity);
        }

        return;
    }

    // The clock starts here, not when the peer gets round to answering.
    _pendingAuth.push_back(PendingJoin{peer, PlayerIdentity{}, Engine::Time::GetTicks() + AUTH_TIMEOUT_MS});

    OutputByteStream hello;
    hello.Write(PT_Hello);
    _transport->SendTo(peer, hello, true);
}

// Spawn a player for the peer, and bring it up to date with the world.
void NetCode::NetworkManager::OnboardNewPlayer(const PeerID peer)
{
    if (!HasWorldAuthority()) return;

    Engine::Level* level = Engine::LevelManager::GetCurrentLevel();
    if (level == nullptr) return;

    Engine::GameObject* pawn = level->GetGameMode().SpawnPlayer(peer);
    _state = NMS_Playing;

    DEBUG_LOG("Netcode: onboarded peer %llu -- pawn %s", peer, pawn != nullptr ? "spawned" : "NOT SPAWNED")

    // A dedicated server is not a peer of its own, so it never takes this branch for
    // itself -- which is exactly why it ends up with no pawn of its own.
    if (peer == _transport->GetLocalPeerID()) return;

    // The slot is opened before the snapshot is written, because the snapshot is
    // filtered to what this peer can see and that needs its pawn as a viewpoint.
    level->AddReplicationPeer(peer, pawn);

    OutputByteStream worldState;
    worldState.Write(PT_WorldState);
    level->WriteCompleteStateFor(peer, worldState);

    if (!_transport->SendTo(peer, worldState, true))
    {
        DEBUG_LOG("Netcode: could not send the world to peer %llu; dropping it.", peer)
        level->RemoveReplicationPeer(peer);
    }
}

void NetCode::NetworkManager::HandlePeerDisconnected(const PeerID peer)
{
    // Ended for anyone Steam is tracking, whether they were admitted or still pending.
    // Skipping it leaves Steam believing the account is still on this server.
    if (const PlayerIdentity* identity = FindIdentity(peer))
        SteamGameServerAuth::Get().EndAuth(identity->id);

    for (auto it = _pendingAuth.begin(); it != _pendingAuth.end();)
    {
        if (it->peer != peer) { ++it; continue; }
        if (it->identity.IsValid())
            SteamGameServerAuth::Get().EndAuth(it->identity.id);
        it = _pendingAuth.erase(it);
    }

    // Already gone, so there is nothing left to drop.
    std::erase_if(_pendingKicks, [peer](const PendingKick& kick) { return kick.peer == peer; });

    Engine::Level* level = Engine::LevelManager::GetCurrentLevel();
    if (level == nullptr) return;

    // Freed before the pawn is removed, so the departing peer is not queued a
    // destruction record for its own pawn -- and so its slot is available again to
    // whoever connects next.
    level->RemoveReplicationPeer(peer);

    if (const auto player = FindPlayerController(peer))
        level->RemoveGameObject(player->gameObject);

    if (const PlayerIdentity* identity = FindIdentity(peer))
        level->GetGameMode().OnPlayerLeft(peer, *identity);

    std::erase_if(_players, [peer](const ConnectedPlayer& player) { return player.peer == peer; });

    DEBUG_LOG("Player %llu disconnected", peer)
}
