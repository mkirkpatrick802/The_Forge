#pragma once
#include <vector>
#include "Object.h"
#include "json.hpp"

namespace Engine
{
    class Component;
    class GameObject : public Object
    {
        friend class Level;
    public:

        GameObject();
        GameObject(const nlohmann::json& data);
        ~GameObject();
        
        template <class T>
        T* GetComponent() const;
        
        std::vector<Component*> GetAllComponents() const { return _attachedComponents; }
        
        void AddComponent(Component* component);

    private:
        
        nlohmann::json SaveObject() override;
        
    public:
        
        std::vector<Component*> _attachedComponents;

        // Only contains the default data used when the game object is first created
        // TODO: Can be moved to a utils file
        nlohmann::json _defaultData;
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
