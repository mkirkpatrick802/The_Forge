//
// Created by mKirkpatrick on 2/20/2024.
//

#ifndef THE_FORGE_PLAYERCONTROLLER_H
#define THE_FORGE_PLAYERCONTROLLER_H

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

    void Update(float deltaTime);

    uint8 GetPlayerID() const { return _playerID; }

private:

    uint8 _playerID;
    InputManager* _inputManager;
};


#endif //THE_FORGE_PLAYERCONTROLLER_H
