#pragma once
#include "Engine/Components/ComponentUtils.h"

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
    
    void Deserialize(const json& data) override;
    nlohmann::json Serialize() override;

private:
    void CollectInputs();
    void EnableWrench(Engine::GameObject* target, glm::vec2 mousePos, WrenchState state);
    void DisableWrench();
    
private:
    WrenchState _currentState = WS_Off;
    ResourceManager* _resourceManager = nullptr;
    float _damage = 10.f;
    float _range = 30.f;
    
};

REGISTER_COMPONENT(FluxWrench)
