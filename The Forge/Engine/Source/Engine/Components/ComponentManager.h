#pragma once
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "ComponentPool.h"
#include "ComponentRegistry.h"

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

        template <typename T>
        std::vector<T*> GetAllComponents();

        template <typename T>
        void DeleteComponent(Component* component);

    private:
        std::unordered_map<std::type_index, std::unique_ptr<BasePool>> _componentPools;
        
    };

    template <typename T>
    void ComponentManager::RegisterComponentPool(const std::string& name)
    {
        // Create pools if it doesn't exist
        if (const auto type = std::type_index(typeid(T)); !_componentPools.contains(type))
            _componentPools[type] = std::make_unique<ComponentPool<T>>();
    }

    template <typename T>
    ComponentPool<T>* ComponentManager::GetPool()
    {
        const auto typeId = std::type_index(typeid(T));
        if (const auto it = _componentPools.find(typeId); it != _componentPools.end()) 
            return static_cast<ComponentPool<T>*>(it->second.get());
        
        return nullptr;
    }
    
    template <typename T>
    std::vector<T*> ComponentManager::GetAllComponents()
    {
        std::vector<T*> results;
        
        for (auto& [type, basePool] : _componentPools)
        {
            if (type == typeid(T))
            {
                if (auto* pool = dynamic_cast<ComponentPool<T>*>(basePool.get()))
                {
                    auto activeComponents = pool->GetActive();
                    results.insert(results.end(), activeComponents.begin(), activeComponents.end());
                }
            }
        }

        return results;
    }


    template <typename T>
    void ComponentManager::DeleteComponent(Component* component)
    {
        auto pool = GetPool<T>();
        
        if (pool == nullptr)
            return;
        
        pool->Delete(static_cast<T*>(component));
    }

    inline ComponentManager& GetComponentManager()
    {
        return ComponentManager::GetInstance();
    }
}
