#include "GameModeBase.h"

#include "Level.h"
#include "LevelManager.h"
#include "System.h"
#include "../../../Netcode/Source/NetworkManager.h"
#include "Components/ComponentUtils.h"
#include "Components/PlayerStart.h"

Engine::GameModeBase::GameModeBase()
{
    _playerStarts = GetComponentManager().GetPool<PlayerStart>()->GetActive();
}

void Engine::GameModeBase::Start()
{
    NetCode::GetNetworkManager().StartNetCode();
}

void Engine::GameModeBase::SpawnPlayer()
{
    LevelManager::GetCurrentLevel()->SpawnNewGameObject("Assets/Player.prefab");
}
