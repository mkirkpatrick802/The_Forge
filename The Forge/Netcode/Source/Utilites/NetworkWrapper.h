#pragma once
#include <memory>
#include <string>

#include "NetCodeTypes.h"

namespace NetCode
{
    bool InitNetcodeBackend();
    void ShutdownNetcodeBackend();

    //Create a Lobby
    class NetObject;
    std::shared_ptr<NetObject> CreateLobby(const std::string& name, LobbyType lobbyType = LobbyType::Public, int maxPlayerCount = 4);
    std::shared_ptr<NetObject> JoinLobby(const std::string& name);
}
