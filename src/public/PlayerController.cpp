//
// Created by mKirkpatrick on 2/20/2024.
//

#include "PlayerController.h"
#include <SDL_scancode.h>

#include "ByteStream.h"
#include "Client.h"
#include "InputManager.h"
#include "GameObject.h"

PlayerController::PlayerController()
{

}

void PlayerController::BeginPlay()
{
	Component::BeginPlay();

    // DOESN'T SYNC IF SERVER IS TABBED OUT (IE WINDOW IS PAUSED)
    if (!Client::IsHostClient())
    {
        ByteStream stream;
        stream.WriteGSM(GSM_Client::GSM_SyncWorld);
        Client::SendByteStreamToServer(stream);

        printf("Sending Sync World Request \n");
    }
}

void PlayerController::Update(float deltaTime)
{

    if(_inputManager){
        const int horizontalMovement = _inputManager->GetKey(SDL_SCANCODE_RIGHT) - _inputManager->GetKey(SDL_SCANCODE_LEFT);
        const int verticalMovement = _inputManager->GetKey(SDL_SCANCODE_DOWN) - _inputManager->GetKey(SDL_SCANCODE_UP);

        const Vector2D newPosition = Vector2D(gameObject->GetPosition().x + (float)horizontalMovement * 2, gameObject->GetPosition().y + (float)verticalMovement * 2 * -1);
        gameObject->SetPosition(newPosition);
    }
}

void PlayerController::LoadData(const json &data)
{

}

void PlayerController::SetInputManager(InputManager* inputManager)
{
    _inputManager = inputManager;
}