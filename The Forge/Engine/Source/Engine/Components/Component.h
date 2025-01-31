#pragma once
#include <json.hpp>
#include "Engine/GameObject.h"

using json = nlohmann::json;

namespace Engine
{
    
    class GameObject;
    
    class Component
    {
    public:
        virtual ~Component() = default;

        virtual void LoadData(const json& data) = 0;
        virtual nlohmann::json SaveData() = 0;

        virtual void BeginPlay() {}
        virtual void Update(float deltaTime) {}

        virtual void OnDestroyed() {}
        virtual void CleanUp() = 0;
        
    public:
        
        GameObject* gameObject = nullptr;

    };
}
