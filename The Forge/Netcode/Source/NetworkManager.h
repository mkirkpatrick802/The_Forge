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
}

namespace NetCode
{
    enum NetworkManagerState : uint8_t
    {
        NMS_Unitialized,
        NMS_Searching,
        
        NMS_Starting,
        NMS_Playing,
        NMS_Delay,
    };

    enum PacketType : uint8_t
    {
        PT_Hello = 0,
        PT_WorldState,
        PT_WorldStateUpdate,
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
        
        void EnterLobby(uint64_t lobbyID);
        void UpdateLobbyPlayers();
        
        void OnboardNewPlayer(uint64_t playerID);
        
        bool IsPlayerInGame(uint64_t playerID) const;
        void HandleConnectionReset(uint64_t playerID);

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
        uint64_t _targetStateUpdateDelay = 60; // Starting place (balance accordingly)
        bool _readyToGetUpdates = false;

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
