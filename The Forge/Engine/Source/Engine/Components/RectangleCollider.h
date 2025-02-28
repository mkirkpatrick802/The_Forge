#pragma once
#include "../Collisions/Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class RectangleCollider final : public Collider
    {
    public:
        RectangleCollider();
        bool CheckCollision(const Collider* collider) const override;
        bool CheckCollision(glm::vec2 pos) const override;

    private:
        glm::vec2 _size;

    public:
        glm::vec2 GetSize() const { return _size; }
        
    };

    REGISTER_COMPONENT(RectangleCollider);
}
