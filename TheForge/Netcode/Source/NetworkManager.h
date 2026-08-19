#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "ByteStream.h"
#include "INetTransport.h"

namespace Engine
{
    class GameObject;
    class PlayerController;
}

namespace NetCode
{
    constexpr uint32_t MAX_PACKET_SIZE_BYTES = 1200;
    
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
        PT_Max
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
        void StartNetCode();

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
        bool HasWorldAuthority() const { return _transport->IsAuthority(); }

        // Spawns a pawn for a peer and brings it up to date. Authority only.
        void OnboardNewPlayer(PeerID peer);
        void HandlePeerDisconnected(PeerID peer);

        INetTransport& GetTransport() const { return *_transport; }

    private:
        Engine::PlayerController* FindPlayerController(PeerID peer) const;

    private:
        std::unique_ptr<INetTransport> _transport;
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
