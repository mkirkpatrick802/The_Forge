#include "CircleCollider.h"

#include "RectangleCollider.h"

Engine::CircleCollider::CircleCollider(): _radius(32)
{
    type = EColliderType::ECT_Circle;
}
