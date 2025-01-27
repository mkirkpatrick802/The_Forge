#include "LobbyHost.h"

#include <iostream>
#include <steam/isteammatchmaking.h>

#include "Server.h"
#include "Client/Client.h"

NetCode::LobbyHost::LobbyHost(const std::string& name, const LobbyType lobbyType, const int maxPlayerCount)
{
    SteamMatchmaking()->CreateLobby((ELobbyType)lobbyType, maxPlayerCount);
    SteamMatchmaking()->SetLobbyData(_currentLobby, "name", name.c_str());
    
    _server = std::make_unique<Server>();
    _host = std::make_unique<Client>();
}

NetCode::LobbyHost::~LobbyHost()
{
    _host.reset();
    _server.reset();
}

void NetCode::LobbyHost::Update()
{

    // Net Object Updates Callbacks
    NetObject::Update();
}

void NetCode::LobbyHost::OnLobbyCreated(LobbyCreated_t* pParam)
{
    if (pParam->m_eResult == k_EResultOK)
    {
        _currentLobby = pParam->m_ulSteamIDLobby;
        std::cout << "Lobby created! Lobby ID: " << _currentLobby.ConvertToUint64() << '\n';
    }
    else
    {
        std::cerr << "Failed to create lobby!" << '\n';
    }
}

void NetCode::LobbyHost::OnLobbyChatMessage(LobbyChatMsg_t* pParam)
{
    
}

