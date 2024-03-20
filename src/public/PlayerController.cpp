//
// Created by mKirkpatrick on 2/20/2024.
//

#include "PlayerController.h"
#include <SDL_scancode.h>
#include "InputManager.h"
#include "GameObject.h"

PlayerController::PlayerController() {

}

void PlayerController::Update(float deltaTime) {

    if(_inputManager){
        const int horizontalMovement = _inputManager->GetKey(SDL_SCANCODE_RIGHT) - _inputManager->GetKey(SDL_SCANCODE_LEFT);
        const int verticalMovement = _inputManager->GetKey(SDL_SCANCODE_DOWN) - _inputManager->GetKey(SDL_SCANCODE_UP);

        const Vector2D newPosition = Vector2D(gameObject->GetPosition().x + (float)horizontalMovement * 2, gameObject->GetPosition().y + (float)verticalMovement * 2 * -1);
        gameObject->SetPosition(newPosition);
    }
}

void PlayerController::LoadData(const json &data) {

}

void PlayerController::SetInputManager(struct InputManager *inputManager) {
    _inputManager = inputManager;
}


