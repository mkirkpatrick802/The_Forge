#include "ShipPiece.h"

#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/DebugRenderer.h"

using namespace Engine;

std::string SHIP_MANAGER_PREFAB = "Assets/Prefabs/Ship Manager.prefab";

void ShipPiece::Start()
{
    gameObject->isReplicated = false;

    _collider = gameObject->GetComponent<Collider>();
    if (gameObject->GetParent() == nullptr)
    {
        _collider->SetCollisionResponseToAllObjects(ECollisionResponse::ECR_Overlap);
        _collider->SetCollisionResponseByObject(ECollisionObjectType::ECOT_Player, ECollisionResponse::ECR_Ignore);
    }

    _collider->OnOverlapBegin.Bind(this, &ShipPiece::OnOverlapBegin);
    _collider->OnOverlapEnd.Bind(this, &ShipPiece::OnOverlapEnd);
}

void ShipPiece::Update(float deltaTime)
{
    
}

glm::vec2 ShipPiece::GetNearestSnapLocation(glm::vec2 mouse, const ShipPiece* other) const
{
    glm::vec2 position = gameObject->GetWorldPosition();
    float rotation = gameObject->GetWorldRotation(); // You might need this if you're rotating grid points
    glm::vec2 size = ((RectangleCollider*)_collider)->GetSize();
    glm::vec2 otherSize = other->gameObject->GetComponent<RectangleCollider>()->GetSize();
    
    const std::vector<glm::vec2> gridPoints = GetGridPoints(position, size, otherSize);
    
    // Find the closest snap point to the other piece (mouse position)
    glm::vec2 closestSnapPoint = gridPoints[0];
    float minDistance = std::numeric_limits<float>::max();

    for (const auto& gridPoint : gridPoints)
    {
        const float distance = glm::distance(gridPoint, mouse);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestSnapPoint = gridPoint;
        }
    }

    return closestSnapPoint;
}

std::vector<glm::vec2> ShipPiece::GetGridPoints(const glm::vec2 position, const glm::vec2 size, glm::vec2 otherSize) const
{
    std::vector<glm::vec2> gridPoints;
    glm::vec2 halfSize = size * 0.5f;
    glm::vec2 min = position - halfSize - 16.f;
    glm::vec2 max = position + halfSize + 16.f;

    for (float x = min.x; x <= max.x; x += 16.f)
    {
        for (float y = min.y; y <= max.y; y += 16.f)
        {
            bool isBorderX = (x == min.x || x == max.x);
            bool isBorderY = (y == min.y || y == max.y);

            // Exclude corners
            if ((isBorderX || isBorderY) && !(isBorderX && isBorderY))
            {
                glm::vec2 offset = glm::vec2(x, y);
                gridPoints.emplace_back(offset);
            }
        }
    }

    /*for (auto point : gridPoints)
        GetDebugRenderer().DrawRectangle(point, glm::vec2(8), glm::vec3(.5,1,.5));*/
    
    return gridPoints;
}

bool ShipPiece::Place() const
{
    std::vector<Collider*> others; 
    if (GetCollisionManager().CheckCollisions(_collider, others)) return false;
    
    gameObject->isReplicated = true;
    _collider->SetCollisionResponseByObject(
    ECollisionObjectType::ECOT_Player,
        _collider->GetCollisionProfile().type == ECollisionObjectType::ECOT_Walkable 
            ? ECollisionResponse::ECR_Overlap 
            : ECollisionResponse::ECR_Block
    );

    if (_attachedToShip)
    {
        const auto manager = _attachedToShip->gameObject->GetParent();
        manager->AddChild(gameObject, true);
    }
    
    CreateShip();
    return true;
}

void ShipPiece::CreateShip() const
{
    if (gameObject->GetParent() == nullptr)
    {
        const auto manager = LevelManager::GetCurrentLevel()->SpawnNewGameObject(SHIP_MANAGER_PREFAB, gameObject->GetWorldPosition());
        manager->AddChild(gameObject);
        gameObject->SetPosition(glm::vec2(0.0f, 0.0f));
    }
}

void ShipPiece::OnOverlapBegin(Engine::GameObject* go)
{
    
}

void ShipPiece::OnOverlapEnd(Engine::GameObject* go)
{
    
}
