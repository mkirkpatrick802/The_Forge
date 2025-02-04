#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "ComponentHasher.h"
#include "Engine/GameObject.h"

namespace Engine
{
    class Component;
    class ComponentFactories
    {
    public:
        
        static ComponentFactories& GetInstance();

        template <typename T>
        void RegisterComponentFactories(const std::string& name);

        Component* CreateComponentFromID(uint32_t id, GameObject* obj);

    private:

        std::unordered_map<uint32_t, std::function<Component*(GameObject*)>> _componentFactories;
        
    };

    template <typename T>
    void ComponentFactories::RegisterComponentFactories(const std::string& name)
    {
        // Store a factory function that adds the component
        const uint32_t id = HASH_COMPONENT(name);
        _componentFactories[id] = [](GameObject* obj) -> Component* {
            return obj->AddComponent<T>();
        };
    }
}
