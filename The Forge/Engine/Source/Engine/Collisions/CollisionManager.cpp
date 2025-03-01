#include "CollisionManager.h"

#include <iostream>
#include <memory>

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/RectangleCollider.h"

Engine::CollisionManager& Engine::CollisionManager::GetInstance()
{
    static auto instance = std::make_unique<CollisionManager>();
    return *instance;
}

Engine::CollisionManager::CollisionManager(): _quadTree(0, glm::vec2(-500, 500), glm::vec2(1000, 1000))
{
    
}

void Engine::CollisionManager::DebugRender()
{
    _quadTree.DebugRender();
}

void Engine::CollisionManager::Update()
{
    std::vector<Collider*> colliders;
    
    const auto circleColliders = GetComponentManager().GetAllComponents<CircleCollider>();
    const auto rectangleColliders = GetComponentManager().GetAllComponents<RectangleCollider>();

    // Reserve space to avoid multiple reallocations
    colliders.reserve(circleColliders.size() + rectangleColliders.size());

    // Insert both vectors into the colliders vector
    colliders.insert(colliders.end(), circleColliders.begin(), circleColliders.end());
    colliders.insert(colliders.end(), rectangleColliders.begin(), rectangleColliders.end());
    
    _quadTree.Clear();

    for (const auto collider : colliders)
        _quadTree.Insert(collider);
}

void Engine::CollisionManager::CheckCollisions(const std::vector<Collider*>& colliders)
{
    for (const auto* collider : colliders)
    {
        std::vector<Collider*> possibleCollisions;
        _quadTree.Retrieve(possibleCollisions, collider);

        // Check for collisions with nearby objects
        for (const auto* other : possibleCollisions)
        {
            if (collider != other && collider->CheckCollision(other))
            {
                // Handle collision (e.g., resolve, respond, etc.)
                std::cout << "Collision detected!" << '\n';
            }
        }
    }
}

bool Engine::CollisionManager::CheckCollisions(const glm::vec2 point, std::vector<Collider*>& returnObjects)
{
    bool collision = false;
    std::vector<Collider*> possibleCollisions;
    _quadTree.Retrieve(possibleCollisions, point);
    
    // Check for collisions with nearby objects
    for (auto* other : possibleCollisions)
        if (other->CheckCollision(point))
        {
            returnObjects.emplace_back(other);
            collision = true;
        }

    return collision;
}
