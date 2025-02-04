#pragma once
#include "ComponentFactories.h"
#include "ComponentRegistry.h"
#include "ComponentManager.h"

namespace Engine
{
    template <typename T>
    void RegisterComponentHelper(const std::string& name)
    {
        GetComponentRegistry().RegisterComponent<T>(name);
        GetComponentManager().RegisterComponentPool<T>(name);
        GetComponentFactories().RegisterComponentFactories<T>(name);
    }
}
