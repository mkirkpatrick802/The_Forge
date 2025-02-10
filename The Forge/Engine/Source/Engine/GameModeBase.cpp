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

Engine::GameObject* Engine::GameModeBase::SpawnPlayer(const uint64_t playerID) const
{
    const auto go = LevelManager::GetCurrentLevel()->SpawnNewGameObject("Assets/Player.prefab");
    const Transform transform = _playerStarts.empty() ? Transform() : _playerStarts[0]->gameObject->transform;
    go->transform.position = transform.position;
    go->transform.rotation = transform.rotation;
    go->id 	= playerID;

    return go;
}
