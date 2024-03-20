//
// Created by mKirkpatrick on 2/20/2024.
//

#ifndef THE_FORGE_PLAYERCONTROLLER_H
#define THE_FORGE_PLAYERCONTROLLER_H

#include "Component.h"

class PlayerController : public Component {

public:

    const static uint32 ComponentID = PLAYER_CONTROLLER;

    PlayerController();
    virtual void LoadData(const json& data) override;

    void SetInputManager(class InputManager* inputManager);

    void Update(float deltaTime);

private:

    InputManager* _inputManager;

};


#endif //THE_FORGE_PLAYERCONTROLLER_H
