#include "CollisionManager.h"

#include <memory>

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Components/Rigidbody.h"

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
            if (collider != other && collider->CheckCollision(other))
            {
                // Handle collision (e.g., resolve, respond, etc.)
                glm::vec2 normal = glm::normalize(collider->gameObject->transform.position - other->gameObject->transform.position);
                const auto a = collider->gameObject->GetComponent<Rigidbody>();
                const auto b = other->gameObject->GetComponent<Rigidbody>();
                if (a && b)
                {
                    float pen = 10; // temp
                    ResolveCollision(a, b, normal, pen);
                }
                //std::cout << "Collision detected!" << '\n';
            }
        }
    }
}

void Engine::CollisionManager::ResolveCollision(Rigidbody* a, Rigidbody* b, const glm::vec2 normal, float penetration) const
{
    const glm::vec2 relativeVelocity = a->GetVelocity() - b->GetVelocity();
    const float velocityAlongNormal = glm::dot(relativeVelocity, normal);

    // Compute restitution (bounciness)
    const float e = 0; // Assuming inelastic collision

    // Impulse calculation
    float j = -(1 + e) * velocityAlongNormal;
    j /= a->GetInverseMass() + b->GetInverseMass();

    // Apply impulse
    const glm::vec2 impulse = j * normal;
    if (!a->IsStatic()) a->ApplyImpulse(-impulse);
    if (!b->IsStatic()) b->ApplyImpulse(impulse);

    // **Positional Correction** to prevent objects from sticking inside each other
    if (const float slop = 0.01f; penetration > slop)
    {
        const float percent = 0.8f;
        glm::vec2 correction = (penetration * percent / (a->GetInverseMass() + b->GetInverseMass())) * normal;
        if (!a->IsStatic()) a->gameObject->transform.position = glm::vec2(a->gameObject->transform.position - correction * a->GetInverseMass());
        if (!b->IsStatic()) b->gameObject->transform.position = (b->gameObject->transform.position + correction * b->GetInverseMass());
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
