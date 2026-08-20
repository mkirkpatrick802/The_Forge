#include "GameModeBase.h"

#include <algorithm>
#include <ranges>

#include "Level.h"
#include "LevelManager.h"
#include "LinkingContext.h"
#include "System.h"
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

Engine::GameObject* Engine::GameModeBase::SpawnPlayer(const uint64_t playerID)
{
    const auto go = LevelManager::GetCurrentLevel()->SpawnNewGameObject(_playerPrefab);
    if (go == nullptr)
    {
        // Without this the failure is silent and shows up much later as "no camera":
        // the Camera rides on the player prefab.
        DEBUG_LOG("GameMode: could not spawn the player prefab '%s'", _playerPrefab.c_str())
        return nullptr;
    }

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

std::unordered_map<std::string, Engine::GameModeRegistry::Factory>& Engine::GameModeRegistry::Factories()
{
    static std::unordered_map<std::string, Factory> factories;
    return factories;
}

void Engine::GameModeRegistry::Register(const std::string& name, Factory factory)
{
    Factories()[name] = std::move(factory);
}

std::unique_ptr<Engine::GameModeBase> Engine::GameModeRegistry::Create(const std::string& name)
{
    // An unknown or absent name falls back to the base rather than failing, so a level
    // authored before its mode existed -- or one loaded by a tool that has no game
    // linked in, like the editor -- still comes up.
    if (name.empty())
        return std::make_unique<GameModeBase>();

    const auto it = Factories().find(name);
    if (it == Factories().end())
    {
        DEBUG_LOG("Level names game mode '%s', which is not registered; using the base mode.", name.c_str())
        return std::make_unique<GameModeBase>();
    }

    return it->second();
}

std::vector<std::string> Engine::GameModeRegistry::GetNames()
{
    std::vector<std::string> names;
    names.reserve(Factories().size());

    for (const auto& name : Factories() | std::views::keys)
        names.push_back(name);

    std::ranges::sort(names);

    return names;
}
