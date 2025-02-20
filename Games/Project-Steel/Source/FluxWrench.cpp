#include "FluxWrench.h"

#include <SDL_mouse.h>

#include "Destructible.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"
#include "Engine/Components/BoxCollider.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Components/LineRenderer.h"
#include "Engine/Components/PlayerController.h"

using namespace Engine;

void FluxWrench::Update(float deltaTime)
{
    CollectInputs();
}

void FluxWrench::CollectInputs()
{
    if (auto controller = gameObject->GetComponent<PlayerController>())
        if (!controller->IsLocalPlayer()) return;
    
    glm::vec2 mousePos;
    const bool isRightClick = GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_RIGHT), mousePos);
    const bool isLeftClick = GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_LEFT), mousePos);

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
    if (const auto line = gameObject->GetComponent<LineRenderer>())
    {
        line->SetHidden(false);
        line->SetStartAndEnd(gameObject->transform.position, mousePos);
    }

    _currentState = state;

    switch (_currentState)
    {
    case WS_Mining:
        if (const auto destructible = target->GetComponent<Destructible>())
        {
            destructible->TakeDamage(gameObject, 2);
        }
        break;
    case WS_Repairing:
        break;
    }
}

void FluxWrench::DisableWrench()
{
    if (_currentState == WS_Off) return;

    if (const auto line = gameObject->GetComponent<LineRenderer>())
        line->SetHidden(true);
    
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