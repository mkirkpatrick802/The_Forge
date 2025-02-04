#include "ComponentManager.h"
#include "Engine/GameObject.h"

Engine::ComponentManager& Engine::ComponentManager::GetInstance()
{
    static std::unique_ptr<ComponentManager> instance;
    if (instance == nullptr)
        instance = std::make_unique<ComponentManager>();
    
    return *instance;
}

void Engine::ComponentManager::UpdateComponents(float deltaTime)
{
    
}

/*Engine::Component* Engine::ComponentManager::CreateComponentByID(const uint32_t id, GameObject* obj)
{
    if (const auto it = _componentFactories.find(id); it != _componentFactories.end())
    {
        const auto component = it->second(obj); // Calls stored lambda
        return component;
    }
    
    return nullptr;
}*/
