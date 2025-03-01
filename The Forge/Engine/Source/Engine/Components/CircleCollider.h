#pragma once
#include "../Collisions/Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class CircleCollider final : public Collider
    {
    public:
        CircleCollider();

    private:
        float _radius;

    public:
        float GetRadius() const { return _radius; };
    };

    REGISTER_COMPONENT(CircleCollider)
}
