#pragma once
#include <json.hpp>

#include "ByteStream.h"
#include "Engine/Rendering/ShaderUniformData.h"

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

        virtual void Deserialize(const json& data) {}
        virtual nlohmann::json Serialize() { return nlohmann::json(); }

        // Called before the first update frame for this object
        virtual void Start() {}

        // Called every frame from the renderer
        virtual void Render(const ShaderUniformData& data) {}
        
        // Called every frame from the game engine
        virtual void Update(float deltaTime) {}
        
        // Triggers when component is activated within the pool
        // May not have access to other component on the game object 
        virtual void OnActivation() {}

        virtual void Read(NetCode::InputByteStream& stream) {}
        virtual void Write(NetCode::OutputByteStream& stream) const {}

        virtual void DrawDetails() {}
    
    public:
        GameObject* gameObject = nullptr;
        bool isReplicated = false;
        bool isDirty = false;
        int sortingLayer;
        
    };
}
