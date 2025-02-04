#include "ComponentFactories.h"
#include <memory>

Engine::ComponentFactories& Engine::ComponentFactories::GetInstance()
{
    static std::unique_ptr<ComponentFactories> instance;
    if (instance == nullptr)
        instance = std::make_unique<ComponentFactories>();
    
    return *instance;
}

Engine::Component* Engine::ComponentFactories::CreateComponentFromID(const uint32_t id, GameObject* obj)
{
    if (const auto it = _componentFactories.find(id); it != _componentFactories.end())
    {
        const auto component = it->second(obj); // Calls stored lambda
        return component;
    }
    
    return nullptr;
}
