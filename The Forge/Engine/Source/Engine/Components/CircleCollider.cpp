#include "CircleCollider.h"

#include <glm/glm.hpp>
#include "RectangleCollider.h"

Engine::CircleCollider::CircleCollider(): _radius(32)
{
    type = EColliderType::ECT_Circle;
}
