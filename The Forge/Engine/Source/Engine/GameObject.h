#pragma once
#include <typeindex>
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
        ~GameObject() = default;
        
        template <class T>
        T* GetComponent() const;
        
        std::vector<Component*> GetAllComponents() const;

        template <typename T>
        T* AddComponent();

        Component* AddComponentByID(uint32_t id);

    private:
        
        nlohmann::json SaveObject();
        
    public:

        Transform transform;

    private:

        std::string _name;
        std::unordered_map<std::type_index, Component*> _components;

        // Only contains the default data used when the game object is first created
        // TODO: Can be moved to a utils file
        nlohmann::json _defaultData;

    public:

        inline std::string GetName() const { return _name; }
        inline void SetName(const std::string& name) { _name = name; }
    };
}

#include "GameObject.inl"
