#include "CollisionManager.h"

#include <iostream>
#include <memory>

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Components/Rigidbody.h"
#include <glm/glm.hpp>

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
            if (float pen; collider != other && collider->CheckCollision(other, pen))
            {
                // Handle collision (e.g., resolve, respond, etc.)
                const glm::vec2 normal = glm::normalize(other->gameObject->GetWorldPosition() - collider->gameObject->GetWorldPosition());
                const auto a = collider->gameObject->GetComponent<Rigidbody>();
                const auto b = other->gameObject->GetComponent<Rigidbody>();
                if (a && b)
                {
                    ResolveCollision(a, b, normal, pen);
                }
            }
        }
    }
}

void Engine::CollisionManager::ResolveCollision(Rigidbody* a, Rigidbody* b, const glm::vec2 normal, const float penetration) const
{
    const glm::vec2 relativeVelocity = b->GetVelocity() - a->GetVelocity();
    const float velocityAlongNormal = dot(relativeVelocity, normal);

    // If objects are already separating, do nothing
    if (velocityAlongNormal > 0) return;

    // Compute restitution (bounciness)
    const float e = 0.0f; // Assuming inelastic collision

    // Compute impulse scalar
    float j = -(1 + e) * velocityAlongNormal;
    float invMassSum = a->GetInverseMass() + b->GetInverseMass();
    if (invMassSum == 0.0f) return; // Avoid division by zero

    j /= invMassSum;

    // Apply impulse
    const glm::vec2 impulse = j * normal;
    if (!a->IsStatic()) a->ApplyImpulse(-impulse);
    if (!b->IsStatic()) b->ApplyImpulse(impulse);

    // **Positional Correction (to separate overlapping objects)**
    constexpr float percent = 0.6f;  // Penetration correction percentage
    const glm::vec2 correction = (penetration / invMassSum) * percent * normal;
        
    if (!a->IsStatic()) a->gameObject->SetPosition(a->gameObject->GetWorldPosition() - a->GetInverseMass() * correction);
    if (!b->IsStatic()) b->gameObject->SetPosition(b->gameObject->GetWorldPosition() + b->GetInverseMass() * correction);
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

bool Engine::CollisionManager::CheckCollision(const Collider* collider, std::vector<Collider*>& returnObjects)
{
    bool collision = false;
    std::vector<Collider*> possibleCollisions;
    _quadTree.Retrieve(possibleCollisions, collider);
    
    // Check for collisions with nearby objects
    float pen;
    for (auto* other : possibleCollisions)
        if (other->CheckCollision(collider, pen))
        {
            returnObjects.emplace_back(other);
            collision = true;
        }

    return collision;
}
