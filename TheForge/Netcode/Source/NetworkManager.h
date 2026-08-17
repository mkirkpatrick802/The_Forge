#pragma once
#include <cstdint>
#include <list>
#include <queue>
#include <string>

#include "ByteStream.h"
#include "GamerServices.h"

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
    
    class ReceivedPacket
    {
    public:
        ReceivedPacket(const InputByteStream& inStream, const uint64_t inFromPlayer) : _stream(inStream), _playerID(inFromPlayer) {}

        uint64_t GetFromPlayer() const { return _playerID; }
        InputByteStream& GetByteStream() { return _stream; }

    private:
        InputByteStream	_stream;
        uint64_t _playerID;

    };
    
    class NetworkManager
    {
    public:
        static NetworkManager& GetInstance();
        NetworkManager();
        ~NetworkManager();

        void StartNetCode();

        void Update();

        // Process Incoming Packets
        void ProcessIncomingPackets();
        void ReadIncomingPacketsIntoQueue();
        void ProcessQueuedPackets();
        void ProcessPacket(InputByteStream& stream, uint64_t playerID);

        // Prep World State Update
        void SendWorldStateUpdate();

        // Client -> host input. Clients send their input to the authority, which
        // simulates it and replicates the resulting state back out.
        void SendClientInput();
        void ApplyClientInput(InputByteStream& stream, uint64_t playerID);

        // True when this machine simulates the world: the lobby owner, or any time
        // we aren't in a multiplayer session at all (offline / solo).
        bool HasWorldAuthority() const { return _isOwner || _playerCount <= 1; }

        void EnterLobby(uint64_t lobbyID);
        void UpdateLobbyPlayers();
        
        void OnboardNewPlayer(uint64_t playerID);
        
        bool IsPlayerInGame(uint64_t playerID) const;
        void HandleConnectionReset(uint64_t playerID);

    private:
        Engine::PlayerController* FindPlayerController(uint64_t playerID) const;

    private:
        NetworkManagerState	_state;
        uint64_t _lobbyID;
        uint64_t _ownerID;
        uint64_t _localUserID;
        
        int _playerCount;
        bool _isOwner;
        std::map<uint64_t, std::string> _playerNames;
        std::queue<ReceivedPacket, std::list<ReceivedPacket>> _packetQueue;

        uint64_t _lastUpdateSentTicks = 0;
        uint64_t _lastInputSentTicks = 0;
        // Input is small and latency-sensitive, so it goes out more often than world state.
        uint64_t _targetInputSendDelayMs = 16;
        // Milliseconds between world state updates -- compared against Engine::Time::GetTicks(),
        // which is SDL_GetTicks64(). 33ms is roughly 30Hz.
        uint64_t _targetStateUpdateDelayMs = 33;
        std::unordered_map<uint64_t, std::vector<uint8_t>> _reassemblyBuffer;

    public:
        bool GetIsOwner() const { return _isOwner; }
        uint64_t GetLocalUserID() const { return _localUserID; }
        int GetPlayerCount() const { return _playerCount; }
    };

    inline NetworkManager& GetNetworkManager()
    {
        return NetworkManager::GetInstance();
    }
}
