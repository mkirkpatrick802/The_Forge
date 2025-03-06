#include "Astronaut.h"

#include <iostream>
#include <SDL_mouse.h>

#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "imgui_internal.h"
#include "Engine/Collisions/Collider.h"
#include "Engine/Components/Rigidbody.h"
#include "Engine/Rendering/CameraManager.h"

using namespace Engine;

Astronaut::Astronaut(): _rb(nullptr), _state(EAstronautState::EAS_Walking)
{
    
}

void Astronaut::Start()
{
    PlayerController::Start();

    _rb = gameObject->GetComponent<Rigidbody>();
    if (const auto collider = gameObject->GetComponent<Collider>())
    {
        collider->OnOverlapBegin.Bind(this, &Astronaut::OnColliderBeginOverlap);
        collider->OnOverlapEnd.Bind(this, &Astronaut::OnColliderEndOverlap);
    }
}

void Astronaut::Update(const float deltaTime)
{
    PlayerController::Update(deltaTime);

    CollectInput(deltaTime);
}

void Astronaut::CollectInput(const float deltaTime)
{
    if (!IsLocalPlayer()) return;
    
    // Get movement input
    const glm::vec2 movementInput = {
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_D) - GetInputManager().GetKey(SDL_SCANCODE_A)),
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_W) - GetInputManager().GetKey(SDL_SCANCODE_S))
    };

    Move(movementInput, deltaTime);
    
    // Determine rotation direction (face movement or mouse)
    glm::vec2 targetDirection = movementInput; // Default to movement input direction
    if (glm::vec2 mousePos; GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_LEFT), mousePos) || GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_RIGHT)) || length(movementInput) == 0.0f)
    {
        mousePos = GetCameraManager().ConvertScreenToWorld(mousePos);
        targetDirection = glm::normalize(mousePos - gameObject->GetWorldPosition());
    }

    // Apply rotation if valid
    if (length(targetDirection) > 0.0f)
        gameObject->SetRotation(glm::atan(targetDirection.x, targetDirection.y) * 180.0f / glm::pi<float>());
    
    gameObject->isDirty = true;
}

void Astronaut::Move(const glm::vec2 movement, const float deltaTime)
{
    if (length(movement) <= 0.0f) return;
    
    switch (_state)
    {
    case EAstronautState::EAS_Flying:
        const glm::vec2 accelerationVector = glm::normalize(movement) * _flySpeed;
        _rb->ApplyForce(accelerationVector);
        break;
    case EAstronautState::EAS_Walking:
        const glm::vec2 currentPos = gameObject->GetWorldPosition();
    
        // Calculate the movement vector
        const glm::vec2 movementDirection = glm::normalize(movement); // Normalize movement to avoid faster diagonal movement
        _walkVelocity = movementDirection * _walkSpeed; // Speed in the movement direction
    
        // Update the new position based on velocity and deltaTime
        const glm::vec2 newPos = currentPos + _walkVelocity * deltaTime;
    
        // Set the new position
        gameObject->SetPosition(newPos);
        break;
    }
}

void Astronaut::OnColliderBeginOverlap(const Engine::GameObject* overlappedObject)
{
    if (overlappedObject->GetComponent<Collider>()->GetCollisionProfile().type == ECollisionObjectType::ECOT_Walkable)
    {
        _state = EAstronautState::EAS_Walking;
        _rb->ClearVelocity();
    }
}

void Astronaut::OnColliderEndOverlap(const Engine::GameObject* overlappedObject)
{
    _state = EAstronautState::EAS_Flying;
    _rb->SetVelocity(_walkVelocity * .75f);
    _walkVelocity = glm::vec2(0.0f);
}

void Astronaut::DrawDetails()
{
    ImGui::InputFloat("Fly Speed", &_flySpeed);
    ImGui::InputFloat("Walk Speed", &_walkSpeed);
}

nlohmann::json Astronaut::Serialize()
{
    nlohmann::json data =  PlayerController::Serialize();
    data["Fly Speed"] = _flySpeed;
    data["Walk Speed"] = _walkSpeed;
    return data;
}

void Astronaut::Deserialize(const json& data)
{
    PlayerController::Deserialize(data);
    if (data.contains("Fly Speed"))
        _flySpeed = data["Fly Speed"];

    if (data.contains("Walk Speed"))
        _walkSpeed = data["Walk Speed"];
}
