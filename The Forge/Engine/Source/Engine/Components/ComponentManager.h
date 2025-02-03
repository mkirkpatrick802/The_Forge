#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ComponentPool.h"

#define REGISTER_COMPONENT(TYPE) \
class TYPE##RegisterHelper { \
public: \
TYPE##RegisterHelper() { \
Engine::GetComponentManager()->RegisterComponentPool<TYPE>(); \
} \
}; \
static TYPE##RegisterHelper TYPE##_register_helper;

namespace Engine
{
    class ComponentManager
    {
    public:

        static ComponentManager& GetInstance();
        ComponentManager() = default;
        ~ComponentManager() = default;

        void UpdateComponents(float deltaTime);
        
        template <typename T>
        void RegisterComponentPool();

        template <typename T>
        ComponentPool<T>* GetPool();
        
    private:
        
        std::unordered_map<std::type_index, std::unique_ptr<BasePool>> _componentPools;
        
    };

    template <typename T>
    void ComponentManager::RegisterComponentPool()
    {
        if (const auto typeId = std::type_index(typeid(T)); !_componentPools.contains(typeId)) 
            _componentPools[typeId] = std::make_unique<ComponentPool<T>>();
    }

    template <typename T>
    ComponentPool<T>* ComponentManager::GetPool()
    {
        const auto typeId = std::type_index(typeid(T));
        if (const auto it = _componentPools.find(typeId); it != _componentPools.end()) 
            return static_cast<ComponentPool<T>*>(it->second.get());
        
        return nullptr;
    }

    // Global accessor for the component manager
    inline ComponentManager& GetComponentManager()
    {
        return ComponentManager::GetInstance();
    }
}
