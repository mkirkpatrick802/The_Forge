#pragma once
#include <json.hpp>

using json = nlohmann::json;

namespace Engine
{
    class GameObject;
    class Component
    {
    public:

        virtual void Deserialize(const json& data) = 0;
        virtual nlohmann::json Serialize() = 0;

        // Triggers when component is activated within the pool
        virtual void OnActivation() {};

    public:
        
        GameObject* gameObject = nullptr;

    };
}
