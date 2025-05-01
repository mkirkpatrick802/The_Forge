#include "ComponentFactories.h"
#include "Component.h"
#include <memory>

Engine::ComponentFactories& Engine::ComponentFactories::GetInstance()
{
    static auto instance = std::make_unique<ComponentFactories>();
    return *instance;
}

Engine::Component* Engine::ComponentFactories::CreateComponentFromID(const uint32_t id, GameObject* obj)
{
    if (const auto it = _componentCreationFactories.find(id); it != _componentCreationFactories.end())
    {
        const auto component = it->second(obj);
        return component;
    }
    
    return nullptr;
}

void Engine::ComponentFactories::DeleteComponent(Component* component)
{
    if (const auto it = _componentDeletionFactories.find(typeid(*component)); it != _componentDeletionFactories.end())
    {
        it->second(component);
    }
}
