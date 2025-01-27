#pragma once
#include <memory>
#include <string>
#include <steam/isteammatchmaking.h>
#include <steam/steam_api_common.h>

#include "Utilites/NetCodeTypes.h"
#include "Utilites/NetObject.h"

namespace NetCode
{
    class Server;
    class Client;

    class LobbyHost final : public NetObject
    {
    public:

        LobbyHost(const std::string& name, LobbyType lobbyType = LobbyType::Public, int maxPlayerCount = 4);
        ~LobbyHost() override;

        virtual void Update() override;

    private:

        STEAM_CALLBACK(LobbyHost, OnLobbyCreated, LobbyCreated_t);
        STEAM_CALLBACK(LobbyHost, OnLobbyChatMessage, LobbyChatMsg_t);

    private:
        
    private:

        std::unique_ptr<Client> _host;
        std::unique_ptr<Server> _server;

        CSteamID _currentLobby;
    };
}
