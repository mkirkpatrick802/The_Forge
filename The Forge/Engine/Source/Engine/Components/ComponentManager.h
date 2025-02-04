#pragma once
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "ComponentPool.h"

namespace Engine
{
    class Component;
    class GameObject;
    class ComponentManager
    {
    public:

        static ComponentManager& GetInstance();
        ComponentManager() = default;
        ~ComponentManager() = default;

        void UpdateComponents(float deltaTime);
        
        template <typename T>
        void RegisterComponentPool(const std::string& name);

        template <typename T>
        ComponentPool<T>* GetPool();

    private:
        
        std::unordered_map<std::type_index, std::unique_ptr<BasePool>> _componentPoolsByType;
        
    };

    template <typename T>
    void ComponentManager::RegisterComponentPool(const std::string& name)
    {
        //const uint32_t id = HASH_COMPONENT(name);
        auto pool = std::make_unique<ComponentPool<T>>();
        
        // Create pools if it doesn't exist
        if (const auto type = std::type_index(typeid(T)); !_componentPoolsByType.contains(type))
            _componentPoolsByType[type] = std::move(pool);
    }

    template <typename T>
    ComponentPool<T>* ComponentManager::GetPool()
    {
        const auto typeId = std::type_index(typeid(T));
        if (const auto it = _componentPoolsByType.find(typeId); it != _componentPoolsByType.end()) 
            return static_cast<ComponentPool<T>*>(it->second.get());
        
        return nullptr;
    }
}
