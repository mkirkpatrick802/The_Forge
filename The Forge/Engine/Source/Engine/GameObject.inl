#pragma once
#include <iostream>

#include "Components/ComponentManager.h"

namespace Engine
{
    template <typename T>
    T* GameObject::GetComponent() const
    {
        if (_components.empty()) return nullptr;
        for (const auto& component : _components | std::views::values) {
            if (T* casted = dynamic_cast<T*>(component)) {
                return casted;  // Found a matching derived class
            }
        }
        return nullptr;  // No matching component found
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
