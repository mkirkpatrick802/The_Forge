#include "FluxWrench.h"

#include <SDL_mouse.h>

#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"
#include "Engine/Components/BoxCollider.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Components/LineRenderer.h"

using namespace Engine;

void FluxWrench::Update(float deltaTime)
{
    CollectInputs();
}

void FluxWrench::CollectInputs()
{
    glm::vec2 mousePos;
    const bool isRightClick = GetInputManager().GetButton(SDL_BUTTON_RIGHT, mousePos);
    const bool isLeftClick = GetInputManager().GetButton(SDL_BUTTON_LEFT, mousePos);

    if (isRightClick || isLeftClick)
    {
        mousePos = GetCameraManager().ConvertScreenToWorld(mousePos);
        const auto colliders = GetComponentManager().GetAllComponents<BoxCollider>();

        if (!colliders.empty())
        {
            for (const auto collider : colliders)
            {
                if (collider->CheckCollision(mousePos))
                {
                    EnableWrench(collider->gameObject, mousePos, isRightClick ? WS_Mining : WS_Repairing);
                    return;  // Stop after the first valid click
                }
            }
        }
    }

    DisableWrench();
}

void FluxWrench::EnableWrench(Engine::GameObject* target, glm::vec2 mousePos, WrenchState state)
{
    
}

void FluxWrench::DisableWrench()
{
    if (_currentState == WS_Off) return;

    _currentState = WS_Off;
}

void FluxWrench::Deserialize(const json& data)
{
    
}

nlohmann::json FluxWrench::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<FluxWrench>();
    return data;
}