#pragma once

#include "Component.h"
#include "Shader.h"

class PlayerController : public Component
{
public:

    const static uint32 ComponentID = PLAYER_CONTROLLER;

    PlayerController() = default;

	void InitController(uint8 ID);
    virtual void BeginPlay() override;
    virtual void LoadData(const json& data) override;

    void SetInputManager(class InputManager* inputManager);

    virtual void Update(float deltaTime) override;

    virtual void OnDestroyed() override;

private:

    void ProcessInput();

    void SetShaderUniforms(Shader& shader);

public:

    uint8 playerID;
    float movementSpeed;

private:

    InputManager* _inputManager;
};
