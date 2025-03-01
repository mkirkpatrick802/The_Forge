#include "Collider.h"

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/RectangleCollider.h"

void Engine::Collider::OnActivation()
{
    Component::OnActivation();
}

bool Engine::Collider::CheckCollision(const Collider* collider) const
{
    if (!collider) return false;
    
    if (type == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(this);
        
        if (collider->type == EColliderType::ECT_Circle)
        {
            auto* other = dynamic_cast<const CircleCollider*>(collider);
            return CheckCircleCollision(circle, other);
        }
        
        if (collider->type == EColliderType::ECT_Rectangle)
        {
            const auto* other = dynamic_cast<const RectangleCollider*>(collider);
            return CheckCircleRectangleCollision(circle, other);
        }
    }
    else if (type == EColliderType::ECT_Rectangle)
    {
        const auto* rectangle = dynamic_cast<const RectangleCollider*>(this);
        
        if (collider->type == EColliderType::ECT_Circle)
        {
            const auto* circle = dynamic_cast<const CircleCollider*>(collider);
            return CheckCircleRectangleCollision(circle, rectangle);
        }

        if (collider->type == EColliderType::ECT_Rectangle)
        {
            auto* other = dynamic_cast<const RectangleCollider*>(collider);
            return CheckRectangleCollision(rectangle, other);
        }
    }

    return false;
}

bool Engine::Collider::CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other) const
{
    const auto& otherPos = other->gameObject->transform.position;

    float dx = circle->gameObject->transform.position.x - otherPos.x;
    float dy = circle->gameObject->transform.position.y - otherPos.y;
    float radiusSum = circle->GetRadius() + other->GetRadius();

    return (dx * dx + dy * dy) <= (radiusSum * radiusSum);
}

bool Engine::Collider::CheckCircleRectangleCollision(const CircleCollider* circle,
    const RectangleCollider* rectangle) const
{
    const auto& pos = gameObject->transform.position;
    const auto& otherPos = rectangle->gameObject->transform.position;
    const auto& size = rectangle->GetSize();

    float closestX = std::clamp(pos.x, otherPos.x, otherPos.x + size.x);
    float closestY = std::clamp(pos.y, otherPos.y, otherPos.y + size.y);

    float dx = pos.x - closestX;
    float dy = pos.y - closestY;

    return (dx * dx + dy * dy) <= (circle->GetRadius() * circle->GetRadius());
}

bool Engine::Collider::CheckRectangleCollision(const RectangleCollider* rectangle, const RectangleCollider* other) const
{
    const auto& otherPos = other->gameObject->transform.position;
    const auto& otherSize = other->GetSize();
    const auto& pos = gameObject->transform.position;
    const auto& size = rectangle->GetSize();

    // AABB (Axis-Aligned Bounding Box) collision check
    return (pos.x < otherPos.x + otherSize.x &&
            pos.x + size.x > otherPos.x &&
            pos.y < otherPos.y + otherSize.y &&
            pos.y + size.y > otherPos.y);
}

bool Engine::Collider::CheckCollision(const glm::vec2 pos) const
{
    if (type == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(this);
        float dx = gameObject->transform.position.x - pos.x;
        float dy = gameObject->transform.position.y - pos.y;
        float distanceSquared = dx * dx + dy * dy;

        return distanceSquared <= (circle->GetRadius() * circle->GetRadius());
    }
    
    if (type == EColliderType::ECT_Rectangle)
    {
        const auto* rectangle = dynamic_cast<const RectangleCollider*>(this);
        const auto& rectPos = gameObject->transform.position;
        const auto& size = rectangle->GetSize();

        // Check if the position is within the bounds of the rectangle
        return (pos.x >= rectPos.x && pos.x <= rectPos.x + size.x &&
                pos.y >= rectPos.y && pos.y <= rectPos.y + size.y);
    }

    return false;
}

