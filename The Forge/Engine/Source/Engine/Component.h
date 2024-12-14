#pragma once
#include <json.hpp>

#include "GameObject.h"

#define PLAYER_CONTROLLER (uint32)0
#define SPRITE_RENDERER (uint32)1
#define PROJECTILE (uint32)2
#define COLLIDER (uint32)3
#define HEALTH (uint32)4
#define ENEMY_CONTROLLER (uint32)5

using json = nlohmann::json;

namespace Engine
{
    class GameObject;
    class Component
    {
    public:
        virtual ~Component() = default;

        virtual void LoadData(const json& data) = 0;

        virtual void BeginPlay();
        virtual void Update(float deltaTime);

        virtual void OnDestroyed();

        GameObject* gameObject = nullptr;

    };

    inline void Component::BeginPlay()
    {

    }

    inline void Component::Update(float deltaTime)
    {

    }

    inline void Component::OnDestroyed()
    {

    }
}