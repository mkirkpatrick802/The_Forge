//
// Created by mKirkpatrick on 2/20/2024.
//

#include "PlayerController.h"
#include <SDL_scancode.h>

#include "ByteStream.h"
#include "Client.h"
#include "GameObject.h"
#include "InputManager.h"

void PlayerController::LoadData(const json& data)
{
    movementSpeed = data["Movement Speed"];
}

void PlayerController::BeginPlay()
{
    Component::BeginPlay();
}

void PlayerController::InitController(const uint8 ID)
{
    if (Client::playerID == 255)
        Client::playerID = ID;

    playerID = ID;

    if (!Client::IsHostClient() && playerID == Client::playerID)
    {
        ByteStream stream;
        stream.WriteGSM(GSM_Client::GSM_WorldStateRequest);
        Client::SendByteStreamToServer(stream);
    }
}

void PlayerController::Update(float deltaTime)
{
    ProcessInput();
}

void PlayerController::ProcessInput()
{
    if (!_inputManager || playerID != Client::playerID) return;

    // Movement
    const int8 horizontalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_RIGHT) - _inputManager->GetKey(SDL_SCANCODE_LEFT));
    const int8 verticalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_DOWN) - _inputManager->GetKey(SDL_SCANCODE_UP));
    if (horizontalMovement != 0 || verticalMovement != 0)
    {
        ByteStream stream;
        stream.WriteGSM(GSM_Client::GSM_MovementRequest);
        stream.WritePlayerMovementRequest(gameObject->instanceID, horizontalMovement, verticalMovement);
        Client::SendByteStreamToServer(stream);
    }

    // Shoot
    if(_inputManager->GetKeyDown(SDL_SCANCODE_SPACE))
    {
        ByteStream stream;
        stream.WriteGSM(GSM_Client::GSM_FireRequest);
        stream.WriteFireRequest(gameObject->instanceID);
        Client::SendByteStreamToServer(stream);
    }
}

/*
 *      Setters & Getters
 */

void PlayerController::SetInputManager(InputManager* inputManager)
{
    _inputManager = inputManager;
}