#include "CircleCollider.h"

#include "RectangleCollider.h"

Engine::CircleCollider::CircleCollider(): _radius(32)
{
    type = EColliderType::ECT_Circle;
}
// TODO: move this to base class
bool Engine::CircleCollider::CheckCollision(const Collider* collider) const
{
    // Check if the collider is a CircleCollider
    if (const auto* circle = dynamic_cast<const CircleCollider*>(collider))
    {
        // Circle vs. Circle collision detection
        float dx = gameObject->transform.position.x - circle->gameObject->transform.position.x;
        float dy = gameObject->transform.position.y - circle->gameObject->transform.position.y;
        float distanceSquared = dx * dx + dy * dy;
        float radiusSum = _radius + circle->_radius;

        return distanceSquared <= (radiusSum * radiusSum);
    }
    
    // Check if the collider is a RectangleCollider
    if (const auto* rectangle = dynamic_cast<const RectangleCollider*>(collider))
    {
        // Circle vs. Rectangle collision detection
        float closestX = std::clamp(gameObject->transform.position.x, rectangle->gameObject->transform.position.x, rectangle->gameObject->transform.position.x + rectangle->GetSize().x);
        float closestY = std::clamp(gameObject->transform.position.y, rectangle->gameObject->transform.position.y, rectangle->gameObject->transform.position.y + rectangle->GetSize().y);

        float dx = gameObject->transform.position.x - closestX;
        float dy = gameObject->transform.position.y - closestY;
        float distanceSquared = dx * dx + dy * dy;

        return distanceSquared <= (_radius * _radius);
    }

    return false; // Unknown collider type
}

bool Engine::CircleCollider::CheckCollision(const glm::vec2 pos) const
{
    return glm::distance(gameObject->transform.position, pos) <= _radius;
}
