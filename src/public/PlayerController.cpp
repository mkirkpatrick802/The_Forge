//
// Created by mKirkpatrick on 2/20/2024.
//

#include "PlayerController.h"
#include <SDL_scancode.h>

#include "ByteStream.h"
#include "Client.h"
#include "GameObject.h"
#include "InputManager.h"

void PlayerController::BeginPlay()
{
    Component::BeginPlay();
}

void PlayerController::InitController(const uint8 ID)
{
    _playerID = ID;

    if (!Client::IsHostClient() && _playerID == Client::GetPlayerID())
    {
        ByteStream stream;
        stream.WriteGSM(GSM_Client::GSM_SyncWorld);
        Client::SendByteStreamToServer(stream);
    }
}

void PlayerController::Update(float deltaTime)
{
    if (!_inputManager || _playerID != Client::GetPlayerID()) return;

    const int8 horizontalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_RIGHT) - _inputManager->GetKey(SDL_SCANCODE_LEFT));
    const int8 verticalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_DOWN) - _inputManager->GetKey(SDL_SCANCODE_UP));
    if(horizontalMovement == 0 && verticalMovement == 0) return;

    ByteStream stream;
    stream.WriteGSM(GSM_Client::GSM_MovementRequest);
    stream.WritePlayerMovementRequest(gameObject->instanceID, horizontalMovement, verticalMovement);
    Client::SendByteStreamToServer(stream);
}

void PlayerController::LoadData(const json &data)
{

}

/*
 *      Setters & Getters
 */

void PlayerController::SetInputManager(InputManager* inputManager)
{
    _inputManager = inputManager;
}