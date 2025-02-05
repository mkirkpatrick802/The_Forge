#pragma once
#include <string>
#include "ComponentFactories.h"
#include "ComponentRegistry.h"
#include "ComponentManager.h"

#define REGISTER_COMPONENT(TYPE) \
class TYPE##RegisterHelper { \
public: \
TYPE##RegisterHelper() { \
RegisterComponentHelper<TYPE>(#TYPE); \
} \
}; \
static TYPE##RegisterHelper TYPE##_register_helper;

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