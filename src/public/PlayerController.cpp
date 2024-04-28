
#include "PlayerController.h"
#include <SDL_scancode.h>

#include "ByteStream.h"
#include "Client.h"
#include "GameObject.h"
#include "Health.h"
#include "InputManager.h"
#include "ScoreManager.h"

void PlayerController::LoadData(const json& data)
{
    movementSpeed = data["Movement Speed"];
}

void PlayerController::BeginPlay()
{
    Component::BeginPlay();

    if (gameObject->GetComponent<SpriteRenderer>())
        gameObject->GetComponent<SpriteRenderer>()->RegisterCallback([this](Shader& shader) { this->SetShaderUniforms(shader); });
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

    printf("Init \n");
}

void PlayerController::Update(float deltaTime)
{
    ProcessInput();
}

void PlayerController::OnDestroyed()
{
	Component::OnDestroyed();

    ScoreManager::ResetScore(playerID);
}

void PlayerController::ProcessInput()
{
    if (!_inputManager || playerID != Client::playerID) return;

    // Movement
    const int8 horizontalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_D) - _inputManager->GetKey(SDL_SCANCODE_A));
    const int8 verticalMovement = (int8)(_inputManager->GetKey(SDL_SCANCODE_S) - _inputManager->GetKey(SDL_SCANCODE_W));
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

void PlayerController::SetShaderUniforms(Shader& shader)
{
    if(!gameObject) return;
    if(auto health = gameObject->GetComponent<Health>())
    {
        float percent = health->GetCurrentHealth() / health->GetMaxHealth();
        shader.SetFloat("health", percent);
    }
}

/*
 *      Setters & Getters
 */

void PlayerController::SetInputManager(InputManager* inputManager)
{
    _inputManager = inputManager;
}