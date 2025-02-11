#pragma once
#include <json.hpp>

#include "ByteStream.h"

using json = nlohmann::json;

namespace Engine
{
    class GameObject;
    class Component
    {
        friend class NetCode::InputByteStream;
        friend class NetCode::OutputByteStream;
    public:
        virtual ~Component() = default;

        virtual void Deserialize(const json& data) = 0;
        virtual nlohmann::json Serialize() = 0;

        virtual void Update(float deltaTime) {}
        
        // Triggers when component is activated within the pool
        virtual void OnActivation() {}

        virtual void Read(NetCode::InputByteStream& stream) {}
        virtual void Write(NetCode::OutputByteStream& stream) const {}
    
    public:
        GameObject* gameObject = nullptr;
        bool isReplicated = false;

    };
}
