﻿#include "PlayerController.h"

#include <SDL_mouse.h>

#include "Engine/JsonKeywords.h"
#include <SDL_scancode.h>
#include <glm/ext/scalar_constants.hpp>

#include "NetworkManager.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/System.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/Renderer.h"

void Engine::PlayerController::Deserialize(const json& data)
{
    
}

nlohmann::json Engine::PlayerController::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<PlayerController>();
    return data;
}

void Engine::PlayerController::Update(float deltaTime)
{
    // This should be handled somewheres else
    if (!GetCameraManager().GetActiveCamera())
        if (const auto camera = gameObject->GetComponent<Camera>(); camera && IsLocalPlayer())
            GetCameraManager().SetActiveCamera(camera);
    
    CollectInput(deltaTime);
    UpdatePosition(deltaTime);
}

void Engine::PlayerController::CollectInput(float deltaTime)
{
    if (!IsLocalPlayer()) return;
    
    // Get movement input
    glm::vec2 movementInput = {
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_D) - GetInputManager().GetKey(SDL_SCANCODE_A)),
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_W) - GetInputManager().GetKey(SDL_SCANCODE_S))
    };

    // Apply acceleration if input exists
    if (length(movementInput) > 0.0f) {
        glm::vec2 accelerationVector = glm::normalize(movementInput) * _acceleration * deltaTime;
        _velocity += accelerationVector; // Apply acceleration to velocity
    }

    // Clamp speed to max speed
    if (glm::length(_velocity) > _maxSpeed) {
        _velocity = glm::normalize(_velocity) * _maxSpeed;
    }
    
    // Determine rotation direction (face movement or mouse)
    glm::vec2 targetDirection = movementInput; // Default to movement input direction
    glm::vec2 mousePos;
    if (GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_LEFT), mousePos) || GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_RIGHT)) || length(movementInput) == 0.0f) {
        mousePos = GetCameraManager().ConvertScreenToWorld(mousePos);
        targetDirection = glm::normalize(mousePos - gameObject->transform.position);
    }

    // Apply rotation if valid
    if (glm::length(targetDirection) > 0.0f) {
        gameObject->transform.rotation = (glm::atan(targetDirection.x, targetDirection.y) * 180.0f / glm::pi<float>());
    }
    
    gameObject->isDirty = true;
}

void Engine::PlayerController::UpdatePosition(float deltaTime)
{
    // Apply movement based on velocity
    gameObject->transform.position += _velocity * deltaTime;
}


void Engine::PlayerController::Write(NetCode::OutputByteStream& stream) const
{
    Component::Write(stream);

    stream.Write(_controllingPlayer);
    stream.Write(_velocity);
}

void Engine::PlayerController::Read(NetCode::InputByteStream& stream)
{
    Component::Read(stream);

    stream.Read(_controllingPlayer);
    stream.Read(_velocity);
}

bool Engine::PlayerController::IsLocalPlayer() const
{
    return _controllingPlayer == NetCode::GetNetworkManager().GetLocalUserID();
}
