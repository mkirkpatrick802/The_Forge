#include "RectangleCollider.h"
#include "SpriteRenderer.h"

Engine::RectangleCollider::RectangleCollider(): _size()
{
    type = EColliderType::ECT_Rectangle;
}
