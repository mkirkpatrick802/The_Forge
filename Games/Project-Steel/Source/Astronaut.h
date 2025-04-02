#pragma once
#include "Engine/Components/PlayerController.h"

namespace Engine
{
    class Rigidbody;
}

enum class EAstronautMoveState : uint8_t
{
    EAMS_Flying = 0,
    EAMS_Walking,
};

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
    void CollectInput(float deltaTime);
    void Move(glm::vec2 movement, float deltaTime);
    void FindPositionForShipPiece() const;

    void ToggleBuildMode();
    void PlaceShipPiece();
    
    void OnColliderBeginOverlap(const Engine::GameObject* overlappedObject);
    void OnColliderEndOverlap(const Engine::GameObject* overlappedObject);

    void Write(NetCode::OutputByteStream& stream) const override;
    void Read(NetCode::InputByteStream& stream) override;

private:
    Engine::Rigidbody* _rb;
    EAstronautMoveState _state;
    
    bool _buildMode = false;
    Engine::GameObject* _placementPreview = nullptr;

    float _zoomSpeed = .2f;
    
    // Movement
    float _flySpeed = 600.0f;
    float _walkSpeed = 600.0f;
    glm::vec2 _walkVelocity = glm::vec2(0.0f);

public:
    bool InBuildModeState() const { return _buildMode; }
    
};

REGISTER_COMPONENT(Astronaut)
