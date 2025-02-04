#pragma once
#include <string>

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
    class ComponentRegistry;
    class ComponentManager;
    class ComponentFactories;
    ComponentRegistry& GetComponentRegistry();
    ComponentManager& GetComponentManager();
    ComponentFactories& GetComponentFactories();

    template <typename T>
    void RegisterComponentHelper(const std::string& name);
}

#include "ComponentUtils.inl"