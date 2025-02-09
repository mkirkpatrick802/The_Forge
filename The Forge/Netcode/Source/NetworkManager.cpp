#include "NetworkManager.h"
#include <memory>

#include "GamerServices.h"
#include "Engine/GameModeBase.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"

NetCode::NetworkManager& NetCode::NetworkManager::GetInstance()
{
    static auto instance = std::make_unique<NetworkManager>();
    return *instance;
}

NetCode::NetworkManager::NetworkManager(): _state(NMS_Unitialized), _lobbyID(0), _ownerID(0), _playerCount(0),
                                           _isOwner(false)
{
    _localUserID = GetGamerService().GetLocalPlayerID();
}

NetCode::NetworkManager::~NetworkManager()
{
    
}

void NetCode::NetworkManager::StartNetCode()
{
    // Begin the search for a lobby
    _state = NMS_Searching;
    GetGamerService().LobbySearchAsync();
}

void NetCode::NetworkManager::EnterLobby(const uint64_t lobbyID)
{
    _lobbyID = lobbyID;
    _state = NMS_Starting;
    UpdateLobbyPlayers();
    
    Engine::LevelManager::GetCurrentLevel()->GetGameMode().SpawnPlayer();
}

void NetCode::NetworkManager::UpdateLobbyPlayers()
{
    // We only want to update player counts in lobby before we're starting
    _playerCount = GetGamerService().GetLobbyNumPlayers(_lobbyID);
    _ownerID = GetGamerService().GetOwnerID(_lobbyID);
        
    // Am I the owner now?
    if( _ownerID == _localUserID )
        _isOwner = true;

    GetGamerService().GetLobbyPlayerMap(_lobbyID, _playerNames);
}
