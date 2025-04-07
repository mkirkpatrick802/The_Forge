#include "Astronaut.h"

#include <SDL_mouse.h>

#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "imgui_internal.h"
#include "ShipPiece.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/Collider.h"
#include "Engine/Components/Rigidbody.h"
#include "Engine/Rendering/CameraManager.h"

using namespace Engine;

Astronaut::Astronaut(): _rb(nullptr), _state(EAstronautMoveState::EAMS_Walking)
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

    static bool boundsSet = false;
    if (!boundsSet)
    {
        const auto manager = GetCameraManager();
        if (const auto camera = manager.GetActiveCamera())
        {
            camera->SetZoomBounds({.7, 2});
            boundsSet = true;
        }
    }
    
    CollectInput(deltaTime);
}

void Astronaut::CollectInput(const float deltaTime)
{
    if (!IsLocalPlayer()) return;

    if (const int32_t delta = GetInputManager().GetMouseWheelDelta(); delta != 0)
        (delta > 0) ? GetCameraManager().GetActiveCamera()->ZoomIn(_zoomSpeed) 
                    : GetCameraManager().GetActiveCamera()->ZoomOut(_zoomSpeed);

    
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
        targetDirection = normalize(mousePos - gameObject->GetWorldPosition());
    }

    // Apply rotation if valid
    if (length(targetDirection) > 0.0f)
    {
        gameObject->SetRotation(glm::atan(targetDirection.x, targetDirection.y) * 180.0f / glm::pi<float>());
        gameObject->isDirty = true;
    }

    if (GetInputManager().GetKeyDown(SDL_SCANCODE_B) || (GetInputManager().GetButtonDown(SDL_BUTTON(SDL_BUTTON_RIGHT)) && _buildMode))
        ToggleBuildMode();

    if(_buildMode)
    {
        FindPositionForShipPiece();
        if (GetInputManager().GetButtonDown(SDL_BUTTON(SDL_BUTTON_LEFT)))
            PlaceShipPiece();
    }
}

void Astronaut::Move(const glm::vec2 movement, const float deltaTime)
{
    if (length(movement) <= 0.0f) return;
    
    switch (_state)
    {
    case EAstronautMoveState::EAMS_Flying:
        const glm::vec2 accelerationVector = normalize(movement) * _flySpeed;
        _rb->ApplyForce(accelerationVector);
        break;
    case EAstronautMoveState::EAMS_Walking:
        _rb->ClearVelocity();
        const glm::vec2 currentPos = gameObject->GetWorldPosition();
    
        // Calculate the movement vector
        const glm::vec2 movementDirection = normalize(movement); // Normalize movement to avoid faster diagonal movement
        _walkVelocity = movementDirection * _walkSpeed; // Speed in the movement direction
    
        // Update the new position based on velocity and deltaTime
        const glm::vec2 newPos = currentPos + _walkVelocity * deltaTime;
    
        // Set the new position
        gameObject->SetPosition(newPos);
        break;
    }

    gameObject->isDirty = true;
}

void Astronaut::ToggleBuildMode()
{
    _buildMode = !_buildMode;
    if (_buildMode)
    {
        glm::vec2 pos;
        GetInputManager().GetMousePos(pos);
        _placementPreview = LevelManager::GetCurrentLevel()->SpawnNewGameObject("Assets/Prefabs/Large Hallway Tile.prefab", GetCameraManager().ConvertScreenToWorld(pos));
    }
    else
    {
        Destroy(_placementPreview);
        _placementPreview = nullptr;
    }
}

void Astronaut::FindPositionForShipPiece() const
{
    glm::vec2 pos;
    GetInputManager().GetMousePos(pos);
    pos = GetCameraManager().ConvertScreenToWorld(pos);
    
    if (_placementPreview)
    {
        const auto previewPiece = _placementPreview->GetComponent<ShipPiece>();
        std::vector<Collider*> others;
        GetCollisionManager().CheckCollisions(pos, others);
        if(!others.empty())
        {
            for (const Collider* collider : others)
            {
                if(collider->gameObject == _placementPreview) continue;
                if(const auto piece = collider->gameObject->GetComponent<ShipPiece>())
                {
                    const glm::vec2 snapLocation = piece->GetNearestSnapLocation(pos, previewPiece);
                    previewPiece->SetAttachedToShip(piece);
                    _placementPreview->SetPosition(snapLocation);
                    return;
                }
            }
        }

        
        others.clear();
        _placementPreview->SetPosition(pos);
        GetCollisionManager().CheckCollisions(_placementPreview->GetComponent<Collider>(), others);
        if(!others.empty())
        {
            for (const Collider* collider : others)
            {
                if(collider->gameObject == _placementPreview) continue;
                if(const auto piece = collider->gameObject->GetComponent<ShipPiece>())
                {
                    const glm::vec2 snapLocation = piece->GetNearestSnapLocation(pos, previewPiece);
                    previewPiece->SetAttachedToShip(piece);
                    _placementPreview->SetPosition(snapLocation);
                    return;
                }
            }
        }
        
        previewPiece->SetAttachedToShip(nullptr);
    }
}

void Astronaut::PlaceShipPiece()
{
    if (const auto piece = _placementPreview->GetComponent<ShipPiece>())
        if(!piece->Place())
        {
            return;
        }
    
    _placementPreview = nullptr;
    _placementPreview = LevelManager::GetCurrentLevel()->SpawnNewGameObject("Assets/Prefabs/Large Hallway Tile.prefab");
    FindPositionForShipPiece();
}

void Astronaut::OnColliderBeginOverlap(const Engine::GameObject* overlappedObject)
{
    if (overlappedObject->GetComponent<Collider>()->GetCollisionProfile().type == ECollisionObjectType::ECOT_Walkable)
    {
        _state = EAstronautMoveState::EAMS_Walking;
        _rb->ClearVelocity();
    }
}

void Astronaut::OnColliderEndOverlap(const Engine::GameObject* overlappedObject)
{
    if(std::vector<Collider*> others; !GetCollisionManager().CheckCollisions(gameObject->GetComponent<Collider>(), others))
    {
        for (const auto other : others)
            if(other->GetCollisionProfile().type != ECollisionObjectType::ECOT_Walkable)
                return;
        
        _state = EAstronautMoveState::EAMS_Flying;
        _rb->SetVelocity(_walkVelocity * .75f);
        _walkVelocity = glm::vec2(0.0f);
    }
}

void Astronaut::Write(NetCode::OutputByteStream& stream) const
{
    PlayerController::Write(stream);

    stream.Write(_flySpeed);
    stream.Write(_walkSpeed);
}

void Astronaut::Read(NetCode::InputByteStream& stream)
{
    PlayerController::Read(stream);

    stream.Read(_flySpeed);
    stream.Read(_walkSpeed);
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
