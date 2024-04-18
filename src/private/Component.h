//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_COMPONENT_H
#define THE_FORGE_COMPONENT_H

#include <nlohmann/json.hpp>
#include "GameData.h"

#define PLAYER_CONTROLLER (uint32)0
#define SPRITE_RENDERER (uint32)1

using json = nlohmann::json;

class GameObject;
class Component
{
public:

    virtual void BeginPlay();
    virtual void LoadData(const json& data) = 0;

    GameObject* gameObject;

};

inline void Component::BeginPlay()
{

}

#endif //THE_FORGE_COMPONENT_H
