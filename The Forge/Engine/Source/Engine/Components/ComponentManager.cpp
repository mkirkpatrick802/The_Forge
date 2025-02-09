#include "ComponentManager.h"
#include "Engine/GameObject.h"

#include "PlayerController.h"

Engine::ComponentManager& Engine::ComponentManager::GetInstance()
{
    static auto instance = std::make_unique<ComponentManager>();
    return *instance;
}

void Engine::ComponentManager::UpdateComponents(float deltaTime)
{
    for (const auto& val : _componentPools | std::views::values)
        val->Update(deltaTime);
}
