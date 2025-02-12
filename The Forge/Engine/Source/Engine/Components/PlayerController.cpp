#include "PlayerController.h"

#include "Engine/JsonKeywords.h"
#include <SDL_scancode.h>
#include <glm/ext/scalar_constants.hpp>

#include "NetworkManager.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/System.h"
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
    if (NetCode::GetNetworkManager().GetLocalUserID() != _controllingPlayer) return;
    
    // Get movement input
    const glm::vec2 movementInput = {
        (float)(GetInputManager().GetKey(SDL_SCANCODE_D) - GetInputManager().GetKey(SDL_SCANCODE_A)),
        (float)(GetInputManager().GetKey(SDL_SCANCODE_W) - GetInputManager().GetKey(SDL_SCANCODE_S))
    };

    glm::vec2 mousePos;
    GetInputManager().GetMousePos(mousePos);
    mousePos = Renderer::ConvertScreenToWorld(mousePos);

    glm::vec2 direction = mousePos - gameObject->transform.position;
    direction = glm::normalize(direction);
    float angle = glm::atan(direction.y, direction.x);
    gameObject->transform.rotation = angle * 180/glm::pi<float>() + 90.f;
    
    // Prevent diagonal speed boost by normalizing
    glm::vec2 movement = movementInput;
    if (length(movement) > 0.0f)
    {
        movement = normalize(movement);
    }

    // Apply movement
    gameObject->transform.position += movement * _walkSpeed * deltaTime;
    gameObject->isDirty = true;
}

void Engine::PlayerController::Write(NetCode::OutputByteStream& stream) const
{
    Component::Write(stream);

    stream.Write(_controllingPlayer);
}

void Engine::PlayerController::Read(NetCode::InputByteStream& stream)
{
    Component::Read(stream);

    stream.Read(_controllingPlayer);
}
