#pragma once
#include "Engine/Components/ComponentUtils.h"

namespace Engine
{
    class PlayerController;
}

class ResourceManager;

enum WrenchState : uint8_t
{
    WS_Off = 0,
    WS_Mining,
    WS_Repairing
};

class Engine::GameObject;
class FluxWrench : public Engine::Component
{
public:
    void Start() override;
    void Update(float deltaTime) override;

    void Write(NetCode::OutputByteStream& stream) const override;
    void Read(NetCode::InputByteStream& stream) override;

private:
    void CollectInputs();
    void EnableWrench(Engine::GameObject* target, glm::vec2 mousePos, WrenchState state);
    void DisableWrench();
    void UpdateWrenchVisuals();
    
private:
    WrenchState _currentState = WS_Off;
    ResourceManager* _resourceManager = nullptr;
    Engine::PlayerController* _playerController = nullptr;
    float _damage = 10.f;
    float _range = 30.f;
    glm::vec2 _endpoint = glm::vec2(0);
    
};

REGISTER_COMPONENT(FluxWrench)
