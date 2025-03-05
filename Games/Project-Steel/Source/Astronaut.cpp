#include "Astronaut.h"

#include <SDL_mouse.h>

#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "imgui_internal.h"
#include "Engine/Components/Rigidbody.h"
#include "Engine/Rendering/CameraManager.h"

using namespace Engine;

Astronaut::Astronaut(): rb(nullptr)
{
    
}

void Astronaut::Start()
{
    PlayerController::Start();

    rb = gameObject->GetComponent<Rigidbody>();
}

void Astronaut::Update(const float deltaTime)
{
    PlayerController::Update(deltaTime);

    CollectInput(deltaTime);
}

void Astronaut::CollectInput(float deltaTime) const
{
    if (!IsLocalPlayer()) return;
    
    // Get movement input
    glm::vec2 movementInput = {
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_D) - GetInputManager().GetKey(SDL_SCANCODE_A)),
        static_cast<float>(GetInputManager().GetKey(SDL_SCANCODE_W) - GetInputManager().GetKey(SDL_SCANCODE_S))
    };

    // Apply acceleration if input exists
    if (length(movementInput) > 0.0f)
    {
        const glm::vec2 accelerationVector = glm::normalize(movementInput) * _moveSpeed;
        rb->ApplyForce(accelerationVector);
    }
    
    // Determine rotation direction (face movement or mouse)
    glm::vec2 targetDirection = movementInput; // Default to movement input direction
    glm::vec2 mousePos;
    if (GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_LEFT), mousePos) || GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_RIGHT)) || length(movementInput) == 0.0f)
    {
        mousePos = GetCameraManager().ConvertScreenToWorld(mousePos);
        targetDirection = glm::normalize(mousePos - gameObject->GetWorldPosition());
    }

    // Apply rotation if valid
    if (length(targetDirection) > 0.0f)
        gameObject->SetRotation(glm::atan(targetDirection.x, targetDirection.y) * 180.0f / glm::pi<float>());
    
    gameObject->isDirty = true;
}

void Astronaut::DrawDetails()
{
    ImGui::InputFloat("Move Speed", &_moveSpeed);
}

nlohmann::json Astronaut::Serialize()
{
    nlohmann::json data =  PlayerController::Serialize();
    data["Move Speed"] = _moveSpeed;
    return data;
}

void Astronaut::Deserialize(const json& data)
{
    PlayerController::Deserialize(data);
    if (data.contains("Move Speed"))
        _moveSpeed = data["Move Speed"];
}
