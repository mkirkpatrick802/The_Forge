#include "CollisionManager.h"

#include <iostream>
#include <memory>

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Components/Rigidbody.h"
#include <glm/glm.hpp>

#include "Engine/Level.h"
#include "Engine/LevelManager.h"

Engine::CollisionManager& Engine::CollisionManager::GetInstance()
{
    static auto instance = std::make_unique<CollisionManager>();
    return *instance;
}

Engine::CollisionManager::CollisionManager()
{
    if (const auto level = LevelManager::GetCurrentLevel())
        _quadTree = QuadTree(0, level->GetLevelTopLeft(), level->GetLevelSize());
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

    _quadTree.DebugRender();
}

bool Engine::CollisionManager::CheckCollisions(glm::vec2 point, std::vector<Collider*>& returnObjects,
    ECollisionObjectType objectMask, const std::vector<Collider*>& ignoreColliders)
{
    bool collision = false;
    std::vector<Collider*> possibleCollisions;
    _quadTree.Retrieve(possibleCollisions, point);

    for (auto* other : possibleCollisions)
    {
        // Skip if this collider is in the ignore list
        if (std::ranges::find(ignoreColliders, other) != ignoreColliders.end())
        {
            continue;
        }

        if (other->CheckCollision(point))
        {
            const ECollisionObjectType type = other->GetCollisionProfile().type;
            if (objectMask != ECollisionObjectType::ECOT_None)
            {
                if ((type & objectMask) != ECollisionObjectType::ECOT_None)
                {
                    returnObjects.emplace_back(other);
                    collision = true;
                }
            }
            else
            {
                returnObjects.emplace_back(other);
                collision = true;
            }
        }
    }

    return collision;
}


void Engine::CollisionManager::CheckCollisions(const std::vector<Collider*>& colliders)
{
    for (const auto* collider : colliders)
    {
        if (!collider->GetEnabled()) continue;
        
        std::vector<Collider*> possibleCollisions;
        _quadTree.Retrieve(possibleCollisions, collider);

        // Check for collisions with nearby objects
        for (const auto* other : possibleCollisions)
        {
            if (!other->GetEnabled()) continue;
            
            if (float pen; collider != other && collider->CheckCollision(other, pen))
            {
                // Handle collision (e.g., resolve, respond, etc.)
                // Determine the collision response between the two objects
                if (const ECollisionResponse response = CollisionProfile::ResolveCollision(other->GetCollisionProfile(), collider->GetCollisionProfile()); response == ECollisionResponse::ECR_Overlap)
                {
                    // Overlap begin logic
                    if (!_activeOverlaps[collider].contains(other))
                    {
                        _activeOverlaps[collider].insert(other);
                        collider->OnOverlapBegin.Broadcast(other->gameObject);
                    }

                    // Overlap event (while it's still overlapping)
                    collider->OnOverlap.Broadcast(other->gameObject);
                }
                else
                {
                    // Non-overlap handling (e.g., resolve collision response like block or ignore)
                    const glm::vec2 normal = glm::normalize(other->gameObject->GetWorldPosition() - collider->gameObject->GetWorldPosition());
                    const auto a = collider->gameObject->GetComponent<Rigidbody>();
                    const auto b = other->gameObject->GetComponent<Rigidbody>();
                    if (a && b)
                    {
                        ResolveCollision(a, b, normal, pen);
                    }
                }
            }
            else
            {
                // Check if we need to trigger an "Overlap End" event
                if (_activeOverlaps[collider].contains(other))
                {
                    _activeOverlaps[collider].erase(other);
                    collider->OnOverlapEnd.Broadcast(other->gameObject);
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
    const float e = 0.1f;

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


bool Engine::CollisionManager::CheckCollisions(const Collider* collider, std::vector<Collider*>& returnObjects)
{
    bool collision = false;
    std::vector<Collider*> possibleCollisions;
    _quadTree.Retrieve(possibleCollisions, collider);
    
    // Check for collisions with nearby objects
    float pen;
    for (auto* other : possibleCollisions)
    {
        if (other == collider) continue;
        if (other->CheckCollision(collider, pen))
        {
            returnObjects.emplace_back(other);
            collision = true;
        }
    }

    return collision;
}
