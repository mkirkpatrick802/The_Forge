#include "PlayerController.h"

#include "Engine/JsonKeywords.h"
#include <SDL_scancode.h>
#include <glm/geometric.hpp>

#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"

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
    // Get movement input
    const glm::vec2 movementInput = {
        (float)(GetInputManager().GetKey(SDL_SCANCODE_D) - GetInputManager().GetKey(SDL_SCANCODE_A)),
        (float)(GetInputManager().GetKey(SDL_SCANCODE_W) - GetInputManager().GetKey(SDL_SCANCODE_S))
    };

    // Prevent diagonal speed boost by normalizing
    glm::vec2 movement = movementInput;
    if (length(movement) > 0.0f)
    {
        movement = normalize(movement);
    }

    // Apply movement
    gameObject->transform.position += movement * _walkSpeed * deltaTime;
}
