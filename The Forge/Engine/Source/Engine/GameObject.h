#pragma once
#include <typeindex>
#include <vector>

#include "json.hpp"
#include "ByteStream.h"
#include "NetworkManager.h"

namespace Engine
{
    class Component;
    class GameObject
    {
        friend class Level;
        friend class NetCode::NetworkManager;
        friend class Transform;
        
    public:
        GameObject();
        GameObject(const nlohmann::json& data);
        ~GameObject();

        // Component Logic
        template <class T>
        T* GetComponent() const;
        
        std::vector<Component*> GetAllComponents() const;

        template <typename T>
        T* AddComponent();

        void RemoveComponent(Component* component);
        
        // Parent-Children
        void AddChild(GameObject* child, bool keepWorldPosition = false);
        void RemoveChild(GameObject* child);
        
        // Serialization
        void Deserialize(const nlohmann::json& data);
        nlohmann::json Serialize();
    
    private:
        void UpdateWorldTransform() const;
        
        void Write(NetCode::OutputByteStream& stream) const;
        void Read(NetCode::InputByteStream& stream);
        
    public:
        bool isReplicated = true;
        bool isServerOnly = false;
        bool isDirty = false;
        
        std::string filepath;
        
    private:
        GameObject* _parent;
        std::vector<GameObject*> _children;
        std::string _name;
        std::unordered_map<std::type_index, Component*> _components;

        // Only contains the default data used when the game object is first created
        // TODO: Can be moved to a utils file
        nlohmann::json _defaultData;

    public:
        void SetName(const std::string& name) { _name = name; }
        std::string GetName() const { return _name; }

        void SetPosition(const glm::vec2& position) const;
        glm::vec2 GetWorldPosition() const;
        glm::vec2 GetLocalPosition() const;

        void SetRotation(float rotation) const;
        float GetWorldRotation() const;

        std::vector<GameObject*> GetChildren() const { return _children; }
        GameObject* GetParent() const { return _parent; }
        
    };
}

#include "GameObject.inl"
