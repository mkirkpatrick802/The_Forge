#pragma once
#include <cstdint>
#include <string>

#include "GamerServices.h"

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
    
    class NetworkManager
    {
    public:
        static NetworkManager& GetInstance();
        NetworkManager();
        ~NetworkManager();

        void StartNetCode();

        void EnterLobby(uint64_t lobbyID);
        void UpdateLobbyPlayers();
    
    private:
        NetworkManagerState	_state;
        uint64_t _lobbyID;
        uint64_t _ownerID;
        uint64_t _localUserID;
        
        int _playerCount;
        bool _isOwner;
        std::map<uint64_t, std::string> _playerNames;
    };

    inline NetworkManager& GetNetworkManager()
    {
        return NetworkManager::GetInstance();
    }
}
