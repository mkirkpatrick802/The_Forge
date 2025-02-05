#include "ComponentManager.h"
#include "Engine/GameObject.h"

Engine::ComponentManager& Engine::ComponentManager::GetInstance()
{
    static auto instance = std::make_unique<ComponentManager>();
    return *instance;
}

void Engine::ComponentManager::UpdateComponents(float deltaTime)
{
    
}
