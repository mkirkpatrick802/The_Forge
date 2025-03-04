#pragma once
#include "Engine/Components/PlayerController.h"

namespace Engine
{
    class Rigidbody;
}

class Astronaut : public Engine::PlayerController
{
public:
    Astronaut();
    void Start() override;
    void Update(float deltaTime) override;
    void DrawDetails() override;
    
    nlohmann::json Serialize() override;
    void Deserialize(const json& data) override;

private:
    void CollectInput(float deltaTime) const;

private:
    Engine::Rigidbody* rb;

    // Movement
    float _moveSpeed = 600.0f;
};

REGISTER_COMPONENT(Astronaut)
