#include "CollisionManager.h"

#include <iostream>
#include <limits>
#include <memory>

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/PolygonCollider.h"
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
    const auto polygonColliders = GetComponentManager().GetAllComponents<PolygonCollider>();

    // Reserve space to avoid multiple reallocations
    colliders.reserve(circleColliders.size() + rectangleColliders.size() + polygonColliders.size());

    // Insert each vector into the colliders vector
    colliders.insert(colliders.end(), circleColliders.begin(), circleColliders.end());
    colliders.insert(colliders.end(), rectangleColliders.begin(), rectangleColliders.end());
    colliders.insert(colliders.end(), polygonColliders.begin(), polygonColliders.end());
    
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
                    const glm::vec2 separation = other->GetCenter() - collider->GetCenter();

                    // Two colliders sharing a centre give no direction to push along, and
                    // normalizing that is a divide by zero -- which produces NaN
                    // positions that then propagate through everything they touch.
                    if (dot(separation, separation) <= std::numeric_limits<float>::epsilon())
                        continue;

                    ResolveCollision(collider, other, normalize(separation), pen);
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

namespace
{
    // How much a body yields. Zero -- immovable -- for a body that is marked static, and
    // for one that is not there at all.
    //
    // The absent case is the point. Resolution used to require a Rigidbody on *both*
    // sides and silently do nothing otherwise, so a collider on plain level geometry --
    // a wall, a hull plate, the top half of a cockpit -- was detected, matched, found to
    // Block, and then ignored. It read as "that collider does not work", when what did
    // not work was blocking against anything without a Rigidbody.
    float InverseMassOf(const Engine::Rigidbody* body)
    {
        if (body == nullptr || body->IsStatic()) return 0.0f;
        return body->GetInverseMass();
    }

    glm::vec2 VelocityOf(const Engine::Rigidbody* body)
    {
        return body != nullptr ? body->GetVelocity() : glm::vec2(0.0f);
    }
}

void Engine::CollisionManager::ResolveCollision(const Collider* a, const Collider* b, const glm::vec2 normal, const float penetration) const
{
    GameObject* objectA = a->gameObject;
    GameObject* objectB = b->gameObject;
    if (objectA == nullptr || objectB == nullptr) return;

    // A child and its own parent overlapping is how a multi-part object is *built*, not
    // a collision to push apart -- resolving it would drive the two halves off each
    // other every frame.
    if (objectA->GetParent() == objectB || objectB->GetParent() == objectA) return;

    Rigidbody* bodyA = objectA->GetComponent<Rigidbody>();
    Rigidbody* bodyB = objectB->GetComponent<Rigidbody>();

    const float inverseMassA = InverseMassOf(bodyA);
    const float inverseMassB = InverseMassOf(bodyB);
    const float inverseMassSum = inverseMassA + inverseMassB;

    // Two immovable things cannot be separated, and neither can push the other.
    if (inverseMassSum == 0.0f) return;

    // Normal points from A to B, so a negative closing speed means they are approaching.
    if (const float velocityAlongNormal = dot(VelocityOf(bodyB) - VelocityOf(bodyA), normal); velocityAlongNormal <= 0.0f)
    {
        constexpr float restitution = 0.1f;
        const glm::vec2 impulse = (-(1.0f + restitution) * velocityAlongNormal / inverseMassSum) * normal;

        if (inverseMassA > 0.0f) bodyA->ApplyImpulse(-impulse);
        if (inverseMassB > 0.0f) bodyB->ApplyImpulse(impulse);
    }

    // **Positional Correction (to separate overlapping objects)**
    //
    // Shared out by how much each body yields, so all of it lands on the moving body
    // when the other is a wall.
    constexpr float percent = 0.6f;  // Penetration correction percentage
    const glm::vec2 correction = (penetration / inverseMassSum) * percent * normal;

    if (inverseMassA > 0.0f) objectA->SetWorldPosition(objectA->GetWorldPosition() - inverseMassA * correction);
    if (inverseMassB > 0.0f) objectB->SetWorldPosition(objectB->GetWorldPosition() + inverseMassB * correction);
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
