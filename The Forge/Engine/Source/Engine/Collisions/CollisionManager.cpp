#include "CollisionManager.h"

#include <iostream>
#include <memory>

#include "Engine/Components/ComponentManager.h"

Engine::CollisionManager& Engine::CollisionManager::GetInstance()
{
    static auto instance = std::make_unique<CollisionManager>();
    return *instance;
}

Engine::CollisionManager::CollisionManager(): _quadTree(0, glm::vec2(0, 0), glm::vec2(5000, 5000))
{
    
}

void Engine::CollisionManager::Update()
{
    const auto colliders = GetComponentManager().GetAllComponents<Collider>();
    _quadTree.Clear();

    for (const auto collider : colliders)
        _quadTree.Insert(collider);

    CheckCollisions(colliders);
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
