#pragma once
#include <nlohmann/json.hpp>
#include "GameData.h"

#define PLAYER_CONTROLLER (uint32)0
#define SPRITE_RENDERER (uint32)1
#define PROJECTILE (uint32)2

using json = nlohmann::json;

class GameObject;
class Component
{
public:

    virtual void LoadData(const json& data) = 0;

    virtual void BeginPlay();
    virtual void Update(float deltaTime);

    GameObject* gameObject = nullptr;

};

inline void Component::BeginPlay()
{

}

inline void Component::Update(float deltaTime)
{

}