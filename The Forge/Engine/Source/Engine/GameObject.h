#pragma once
#include <typeindex>
#include <vector>

#include "json.hpp"
#include "Transform.h"
#include "ByteStream.h"
#include "NetworkManager.h"

namespace Engine
{
    class Component;
    class GameObject
    {
        friend class Level;
        friend class NetCode::NetworkManager;
        
    public:
        GameObject();
        GameObject(const nlohmann::json& data);
        ~GameObject();
        
        template <class T>
        T* GetComponent() const;
        
        std::vector<Component*> GetAllComponents() const;

        template <typename T>
        T* AddComponent();

        void Deserialize(const nlohmann::json& data);
        nlohmann::json Serialize();
    
    private:
        void Write(NetCode::OutputByteStream& stream);
        void Read(NetCode::InputByteStream& stream);
        
    public:
        Transform transform;
        uint64_t id;
        bool isReplicated = true;
        bool isDirty = false;
        
        std::string filepath;
        
    private:
        std::string _name;
        std::unordered_map<std::type_index, Component*> _components;

        // Only contains the default data used when the game object is first created
        // TODO: Can be moved to a utils file
        nlohmann::json _defaultData;

    public:
        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { _name = name; }
        
    };
}

#include "GameObject.inl"
