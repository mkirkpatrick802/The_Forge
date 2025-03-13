#include "ShipPiece.h"

#include <glm/detail/func_geometric.inl>

#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/RectangleCollider.h"

using namespace Engine;

std::string SHIP_MANAGER_PREFAB = "Assets/Prefabs/Ship Manager.prefab";

void ShipPiece::Start()
{
    gameObject->isReplicated = false;
    _collider = gameObject->GetComponent<Collider>();
    _collider->SetCollisionResponseToAllObjects(ECollisionResponse::ECR_Overlap);
    _collider->SetCollisionResponseByObject(ECollisionObjectType::ECOT_Player, ECollisionResponse::ECR_Ignore);

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

    const std::vector<glm::vec2> gridPoints = GetGridPoints(position, size);
    
    // Find the closest snap point to the other piece (mouse position)
    glm::vec2 closestSnapPoint = gridPoints[0];
    float minDistance = std::numeric_limits<float>::max();

    for (const auto& gridPoint : gridPoints)
    {
        const float distance = glm::distance(gridPoint + position, mouse);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestSnapPoint = gridPoint;
        }
    }

    // Return the position of the closest snap point as an offset
    const glm::vec2 otherHalfSize = ((RectangleCollider*)other->gameObject->GetComponent<Collider>())->GetSize() * 0.5f;

    // Check which axis is 0 and adjust by adding or subtracting the otherHalfSize
    if (closestSnapPoint.x == 0.0f)
    {
        closestSnapPoint.y += (closestSnapPoint.y < 0 ? -otherHalfSize.y : otherHalfSize.y);
    }
    else if (closestSnapPoint.y == 0.0f)
    {
        closestSnapPoint.x += (closestSnapPoint.x < 0 ? -otherHalfSize.x : otherHalfSize.x);
    }

    return closestSnapPoint + position;
}

std::vector<glm::vec2> ShipPiece::GetGridPoints(const glm::vec2 position, const glm::vec2 size) const
{
    std::vector<glm::vec2> gridPoints;
    
    // Calculate the bounds of the ship piece
    glm::vec2 halfSize = size * 0.5f;
    glm::vec2 min = position - halfSize;
    glm::vec2 max = position + halfSize;

    // Generate grid points as offsets within the bounds of the ship piece
    for (float x = min.x; x <= max.x; x += 16.0f)
    {
        for (float y = min.y; y <= max.y; y += 16.0f)
        {
            glm::vec2 offset(x - position.x, y - position.y);
            if (offset.x == 0 && offset.y == 0) continue;
            if (offset.x != 0 && offset.y != 0) continue;
                
            gridPoints.emplace_back(offset);
        }
    }

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
    
    if (gameObject->GetParent() == nullptr)
    {
        const auto manager = LevelManager::GetCurrentLevel()->SpawnNewGameObject(SHIP_MANAGER_PREFAB, gameObject->GetWorldPosition());
        manager->AddChild(gameObject);
        gameObject->SetPosition(glm::vec2(0.0f, 0.0f));
    }

    return true;
}

void ShipPiece::OnOverlapBegin(Engine::GameObject* go)
{
    
}

void ShipPiece::OnOverlapEnd(Engine::GameObject* go)
{
    
}
