#pragma once
#include "../Collisions/Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class RectangleCollider final : public Collider
    {
    public:
        RectangleCollider();

    private:
        glm::vec2 _size;

    public:
        glm::vec2 GetSize() const { return _size; }
        
    };

    REGISTER_COMPONENT(RectangleCollider);
}
