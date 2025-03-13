#include "FluxWrench.h"

#include <SDL_mouse.h>

#include "Astronaut.h"
#include "Destructible.h"
#include "ResourceManager.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Components/LineRenderer.h"
#include "Engine/Components/PlayerController.h"

using namespace Engine;

void FluxWrench::Start()
{
    _resourceManager = gameObject->GetComponent<ResourceManager>();
    _playerController = gameObject->GetComponent<PlayerController>();
    _astronaut = gameObject->GetComponent<Astronaut>();
}

void FluxWrench::Update(float deltaTime)
{
    if (!_playerController) return;
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
        if (_astronaut->InBuildModeState()) return;
        
        mousePos = GetCameraManager().ConvertScreenToWorld(mousePos);
        if (distance(mousePos, gameObject->GetWorldPosition()) < _range)
        {
            if (std::vector<Collider*> hitColliders; GetCollisionManager().CheckCollisions(mousePos, hitColliders))
            {
                EnableWrench(hitColliders[0]->gameObject, mousePos, isRightClick ? WS_Mining : WS_Repairing);
                return;  // Stop after the first valid click
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
            destructible->TakeDamage(gameObject, _damage, resourceGain);
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
        line->SetStartAndEnd(gameObject->GetWorldPosition(), _endpoint);
    }
}

void FluxWrench::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_currentState);
    stream.Write(_endpoint);
}

void FluxWrench::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_currentState);
    stream.Read(_endpoint);
}