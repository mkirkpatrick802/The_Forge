#include "NetworkWrapper.h"

#include "Client/Client.h"
#include "Server/Server.h"
#include "Server/LobbyHost.h"
#include "steam/steam_api.h"


bool NetCode::InitNetcodeBackend()
{
    SteamAPI_RestartAppIfNecessary(480);
    return SteamAPI_Init();
}

void NetCode::ShutdownNetcodeBackend()
{
    SteamAPI_Shutdown();
}

std::shared_ptr<NetCode::NetObject> NetCode::CreateLobby(const std::string& name, LobbyType lobbyType, int maxPlayerCount)
{
    return std::make_shared<LobbyHost>(name, lobbyType, maxPlayerCount);
}

std::shared_ptr<NetCode::NetObject> NetCode::JoinLobby(const std::string& name)
{
    return std::make_shared<Client>(name);
}
