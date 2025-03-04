#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    class Rigidbody final : public Component
    {
    public:
        Rigidbody();
        void OnActivation() override;

        void Update(float deltaTime) override;
        void ApplyForce(glm::vec2 force);
        void ApplyImpulse(glm::vec2 impulse);
        
    private:
        glm::vec2 _velocity;
        glm::vec2 _acceleration;
        
        float _mass;
        float _inverseMass;

    public:
        glm::vec2 GetVelocity() const { return _velocity; }
        float GetInverseMass() const { return _inverseMass; }
    };

    REGISTER_COMPONENT(Rigidbody)
}
