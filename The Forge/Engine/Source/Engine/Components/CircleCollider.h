#pragma once
#include "../Collisions/Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class CircleCollider final : public Collider
    {
    public:
        CircleCollider();
        bool CheckCollision(const Collider* collider) const override;
        bool CheckCollision(glm::vec2 pos) const override;

    private:
        float _radius;
    };

    REGISTER_COMPONENT(CircleCollider)
}
