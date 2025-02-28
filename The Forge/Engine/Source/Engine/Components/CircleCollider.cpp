#include "CircleCollider.h"

Engine::CircleCollider::CircleCollider(): _radius(32)
{
    type = EColliderType::ECT_Circle;
}

bool Engine::CircleCollider::CheckCollision(const Collider* collider) const
{
    return false;
}

bool Engine::CircleCollider::CheckCollision(const glm::vec2 pos) const
{
    return glm::distance(gameObject->transform.position, pos) <= _radius;
}
