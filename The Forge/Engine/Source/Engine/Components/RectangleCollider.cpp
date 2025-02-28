#include "RectangleCollider.h"
#include "SpriteRenderer.h"

Engine::RectangleCollider::RectangleCollider(): _size()
{
    type = EColliderType::ECT_Rectangle;
}

bool Engine::RectangleCollider::CheckCollision(const Collider* collider) const
{
    return false;
}

bool Engine::RectangleCollider::CheckCollision(const glm::vec2 pos) const
{
    return pos.x >= gameObject->transform.position.x && pos.x <= gameObject->transform.position.x + _size.x &&
               pos.y >= gameObject->transform.position.y && pos.y <= gameObject->transform.position.y + _size.y;
}
