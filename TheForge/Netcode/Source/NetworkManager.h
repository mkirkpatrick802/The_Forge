#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include <vector>

#include "ByteStream.h"
#include "INetTransport.h"
#include "NetActions.h"

namespace Engine
{
    class GameObject;
    class PlayerController;
}

namespace NetCode
{
    constexpr uint32_t MAX_PACKET_SIZE_BYTES = 1200;

    // How long a connected peer may take to prove who it is before being dropped.
    // Generous, because it covers a client asking its own Steam client for a ticket and
    // Steam's backend answering the server -- two round trips to Valve, not to us.
    constexpr uint64_t AUTH_TIMEOUT_MS = 15000;

    // How long a kicked peer is left connected so the reason reaches it.
    //
    // Dropping the connection in the same breath as sending the explanation races it:
    // the goodbye is a bare header and the reason is a reliable payload, and the client
    // tears down on the first thing it processes. Without this, every refusal looks
    // from the player's side like the server silently died.
    constexpr uint64_t KICK_GRACE_MS = 400;
    
    enum NetworkManagerState : uint8_t
    {
        NMS_Unitialized,
        NMS_Searching,
        
        NMS_Starting,
        NMS_Playing,
    };

    enum PacketType : uint8_t
    {
        PT_Hello = 0,
        PT_WorldState,
        PT_WorldStateUpdate,
        PT_ClientInput,

        // Named actions. See NetActions.h for why the two directions are separate
        // packet types rather than one with a direction field: they have different
        // trust properties, and keeping them apart lets the receiving side refuse a
        // message travelling the wrong way before it is parsed at all.
        PT_Request,
        PT_Event,

        PT_Max
    };

    // A peer the authority has accepted, once it has said who it is. Held here rather
    // than on the pawn because a player outlives any particular pawn -- they respawn,
    // and the game mode needs to know who is present before it decides where.
    struct ConnectedPlayer
    {
        PeerID peer = INVALID_PEER;
        PlayerIdentity identity;
    };

    // A peer that has sent its identity and is waiting on Steam's verdict. It is
    // connected but *not admitted*: no pawn, no world state, no replication slot. That
    // is the point -- an unverified peer gets nothing but the chance to prove itself.
    // An identity that is still empty means "connected, has not said who it is yet":
    // the window opens when the peer connects, not when it gets round to answering.
    // Otherwise a peer could hold a connection open indefinitely by staying silent.
    struct PendingJoin
    {
        PeerID peer = INVALID_PEER;
        PlayerIdentity identity;
        uint64_t deadlineTicks = 0;
    };
    
    // Game-side networking: what to send, when to send it, and what arriving
    // messages mean. How any of it reaches the wire is INetTransport's problem.
    //
    // Everything below is transport-agnostic -- there is no Steam or UDP in here,
    // and adding a third transport should not need this file to change.
    class NetworkManager
    {
    public:
        static NetworkManager& GetInstance();
        NetworkManager();
        ~NetworkManager();

        // Joins or hosts a session. Called once the level exists, because onboarding
        // a peer spawns into it -- a transport brought up any earlier could accept a
        // connection with nothing to spawn into.
        //
        // Also where the transport is chosen. That used to happen in the constructor,
        // which was fine while the role could only come from the command line -- but a
        // main menu picks the role and the address at run time, long after this
        // singleton first exists. It is still one decision made once, just once per
        // session rather than once per process.
        void StartNetCode();

        // Where the session has got to. What a loading screen shows while a client works
        // through connect, authenticate and world-state.
        NetworkManagerState GetState() const { return _state; }

        // True once the world has arrived and the game is playable.
        bool IsPlaying() const { return _state == NMS_Playing; }

        // True between StartNetCode and the world arriving -- the window a loading
        // screen covers.
        bool IsConnecting() const { return _state == NMS_Searching || _state == NMS_Starting; }

        // Closes the session. Safe to call twice; the destructor calls it too.
        void ShutdownNetCode();

        void Update();

        void ProcessConnectionEvents();
        void ProcessIncomingPackets();
        void ProcessPacket(InputByteStream& stream, PeerID peer);

        // Prep World State Update
        void SendWorldStateUpdate();

        // Client -> host input. Clients send their input to the authority, which
        // simulates it and replicates the resulting state back out.
        void SendClientInput();
        void ApplyClientInput(InputByteStream& stream, PeerID peer);

        // True when this machine simulates the world. Offline counts: a game with
        // nobody to talk to still has to run itself.
        // Null-guarded because the transport now only exists between StartNetCode and
        // ShutdownNetCode. Before a session, nothing is authoritative over a world that
        // does not exist yet -- and a main menu asking this is exactly that case.
        bool HasWorldAuthority() const { return _transport != nullptr && _transport->IsAuthority(); }

        // Only valid while a session is up. Callers outside one should be asking
        // GetState instead.
        bool HasTransport() const { return _transport != nullptr; }

        // Says hello and opens the window in which a peer must prove who it is. No pawn
        // is spawned and no world state is sent -- that is AdmitPlayer's job, and it
        // only runs once the peer is verified.
        void WelcomePeer(PeerID peer);

        // Spawns a pawn for a peer and brings it up to date. Authority only, and only
        // for a peer whose identity has been established.
        void OnboardNewPlayer(PeerID peer);
        void HandlePeerDisconnected(PeerID peer);

        // --- named actions ---
        // Client -> authority. Returns false if there is no authority to ask, which is
        // the case offline and on the authority itself.
        bool SendRequest(NetActionID id, const OutputByteStream& payload);

        // Authority -> one client, or all of them. No-ops on a non-authority: a client
        // stating facts at other clients is exactly what this design excludes.
        bool SendEventTo(PeerID peer, NetActionID id, const OutputByteStream& payload);
        void BroadcastEvent(NetActionID id, const OutputByteStream& payload);

        // Runs a request through the very handler a client's would reach. For the
        // authority acting on its own behalf -- an editor session, an offline game, a
        // listen-server host -- so that local actions obey exactly the rules remote ones
        // do, rather than a second copy of them that can drift.
        bool DispatchLocalRequest(NetActionID id, const OutputByteStream& payload);

        // Tells one client its request was refused, and why. Worth the round trip --
        // without it a refusal is indistinguishable from a dropped packet.
        void DenyRequest(PeerID peer, const std::string& reason);

        // --- who is connected ---
        const std::vector<ConnectedPlayer>& GetConnectedPlayers() const { return _players; }
        const PlayerIdentity* FindIdentity(PeerID peer) const;
        PeerID FindPeerByPlayerID(uint64_t playerID) const;

        Engine::PlayerController* FindPlayerController(PeerID peer) const;

        INetTransport& GetTransport() const { return *_transport; }

    private:
        // Picks UDP, Steam or Offline from the launch options as they stand now.
        // Destroys any previous transport first, so reconnecting after a disconnect
        // starts from a clean one rather than a shut-down one.
        void SelectTransport();

        // Registered once at construction. Engine-level actions only; a game registers
        // its own against ids from GAME_ACTION_BASE up.
        void RegisterEngineActions();
        void HandleJoinRequest(PeerID from, InputByteStream& payload);

        // Records the identity, spawns the pawn, opens the replication slot and hands
        // over the world. The single point at which a claim becomes a player.
        void AdmitPlayer(PeerID peer, const PlayerIdentity& identity);

        // Steam's verdict on a pending join.
        void OnAuthValidated(uint64_t steamID, bool ok, const char* reason);

        // Kicks anyone who has been pending too long. A client that never answers must
        // not hold a connection open indefinitely.
        void UpdatePendingAuth();

        // Client side: sends Join once Steam has a usable ticket.
        void SendJoinWhenReady();

        void KickPeer(PeerID peer, const std::string& reason);

        PendingJoin* FindPendingJoin(PeerID peer);
        void ErasePendingJoin(PeerID peer);

    private:
        std::unique_ptr<INetTransport> _transport;
        std::vector<ConnectedPlayer> _players;
        std::vector<PendingJoin> _pendingAuth;

        // Peers told why they are going, waiting out KICK_GRACE_MS before the drop.
        struct PendingKick
        {
            PeerID peer = INVALID_PEER;
            uint64_t atTicks = 0;
        };
        std::vector<PendingKick> _pendingKicks;

        // Set on a dedicated server that is not --insecure. When true a claimed identity
        // is never believed on its own.
        bool _requireAuth = false;

        // Client side.
        bool _hasJoined = false;   // Join accepted by the server
        bool _joinSent = false;    // Join is on the wire, waiting for a verdict
        bool _awaitingTicket = false;
        NetworkManagerState	_state = NMS_Unitialized;
        bool _shutdown = false;

        uint64_t _lastUpdateSentTicks = 0;
        uint64_t _lastInputSentTicks = 0;
        // Input is small and latency-sensitive, so it goes out more often than world state.
        uint64_t _targetInputSendDelayMs = 16;
        // Milliseconds between world state updates -- compared against Engine::Time::GetTicks(),
        // which is SDL_GetTicks64(). 33ms is roughly 30Hz.
        uint64_t _targetStateUpdateDelayMs = 33;

    public:
        PeerID GetLocalUserID() const { return _transport->GetLocalPeerID(); }
        bool GetIsOwner() const { return _transport->IsAuthority(); }
    };

    inline NetworkManager& GetNetworkManager()
    {
        return NetworkManager::GetInstance();
    }
}
