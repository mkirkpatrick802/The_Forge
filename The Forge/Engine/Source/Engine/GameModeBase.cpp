#include "GameModeBase.h"

#include "Level.h"
#include "LevelManager.h"
#include "LinkingContext.h"
#include "../../../Netcode/Source/NetworkManager.h"
#include "Components/ComponentUtils.h"
#include "Components/PlayerController.h"
#include "Components/PlayerStart.h"
#include "Components/Transform.h"

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
    if (_playerStarts.empty())
    {
        go->SetPosition(glm::vec2(0));
        go->SetRotation(0);
    }
    else
    {
        const GameObject* start = _playerStarts[0]->gameObject;
        go->SetPosition(start->GetWorldPosition());
        go->SetRotation(start->GetWorldRotation());
    }

    if (const auto controller = go->GetComponent<PlayerController>())
        controller->_controllingPlayer = playerID;
    
    return go;
}
