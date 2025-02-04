#include "ComponentUtils.h"

Engine::ComponentRegistry& Engine::GetComponentRegistry()
{
    return ComponentRegistry::GetInstance();
}

Engine::ComponentManager& Engine::GetComponentManager()
{
    return ComponentManager::GetInstance();
}

Engine::ComponentFactories& Engine::GetComponentFactories()
{
    return ComponentFactories::GetInstance();
}
