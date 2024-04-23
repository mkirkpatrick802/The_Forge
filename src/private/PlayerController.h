//
// Created by mKirkpatrick on 2/20/2024.
//
#pragma once

#include "Component.h"

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

private:

    void ProcessInput();

public:

    uint8 playerID;

private:

    InputManager* _inputManager;
};