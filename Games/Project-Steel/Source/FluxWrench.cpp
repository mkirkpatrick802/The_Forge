#include "FluxWrench.h"

#include <SDL_mouse.h>

#include "Destructible.h"
#include "ResourceManager.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"
#include "Engine/Components/BoxCollider.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Components/LineRenderer.h"
#include "Engine/Components/PlayerController.h"

using namespace Engine;

void FluxWrench::Start()
{
    _resourceManager = gameObject->GetComponent<ResourceManager>();
    _playerController = gameObject->GetComponent<PlayerController>();
}

void FluxWrench::Update(float deltaTime)
{
    if (_playerController->IsLocalPlayer())
    {
        CollectInputs();
    }
    else
    {
        UpdateWrenchVisuals();
    }
}

void FluxWrench::CollectInputs()
{
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
    _currentState = state;
    _endpoint = mousePos;
    UpdateWrenchVisuals();

    switch (_currentState)
    {
    case WS_Mining:
        if (const auto destructible = target->GetComponent<Destructible>())
        {
            int resourceGain = 0;
            destructible->TakeDamage(gameObject, 2, resourceGain);
            if (_resourceManager)
                _resourceManager->GainResources(resourceGain);
        }
        break;
    case WS_Repairing:
        break;
    }
    
    gameObject->isDirty = true;
}

void FluxWrench::DisableWrench()
{
    if (_currentState == WS_Off) return;
    _currentState = WS_Off;

    UpdateWrenchVisuals();
    
    gameObject->isDirty = true;
}

void FluxWrench::UpdateWrenchVisuals() const
{
    if (const auto line = gameObject->GetComponent<LineRenderer>())
    {
        line->SetHidden(_currentState == WS_Off);
        line->SetStartAndEnd(gameObject->transform.position, _endpoint);
    }
}

void FluxWrench::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_currentState);
}

void FluxWrench::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_currentState);
}