#pragma once
#include <vector>
#include "json.hpp"
#include "Transform.h"

namespace Engine
{
    class Component;
    class GameObject
    {
        friend class Level;
    public:

        GameObject();
        GameObject(const nlohmann::json& data);
        
        template <class T>
        T* GetComponent() const;
    
    public:

        String GetName() const { return _name; }

    public:

        Transform transform;
        
    private:
        String _name;
        
        std::vector<Component*> _attachedComponents;
        nlohmann::json _data;
    };

    template <typename T>
    T* GameObject::GetComponent() const
    {
        for (auto component : _attachedComponents) 
        {
            if (T* childComp = dynamic_cast<T*>(component)) 
            {
                return childComp;
            }
        }

        return nullptr;
    }
}
