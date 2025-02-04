#pragma once
#include "Components/ComponentManager.h"
#include "Components/ComponentUtils.h"

namespace Engine
{
    template <typename T>
   T* GameObject::GetComponent() const
    {
        if (const auto it = _components.find(typeid(T)); it != _components.end()) 
        {
            return static_cast<T*>(it->second);
        }
        
        return nullptr;
    }

    template <typename T>
    T* GameObject::AddComponent()
    {
        auto* pool = GetComponentManager().GetPool<T>();
        if (!pool) return nullptr;

        T* component = pool->New(this);
        _components[typeid(T)] = component;  // Store in component list
        return component;
    }   
}
