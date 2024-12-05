#pragma once
#include <vector>

#include "json.hpp"

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

        std::string GetName() const { return _name; }
        
    private:
        std::string _name;
        
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
