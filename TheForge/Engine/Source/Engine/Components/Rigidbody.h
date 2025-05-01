#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    class Rigidbody final : public Component
    {
    public:
        Rigidbody();
        void OnActivation() override;

        void Start() override;
        void Update(float deltaTime) override;
        void ApplyForce(glm::vec2 force);
        void ApplyImpulse(glm::vec2 impulse);

        void DrawDetails() override;

        nlohmann::json Serialize() override;
        void Deserialize(const nlohmann::json& json) override;
        
        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;

    private:
        void CalculateInverseMass();
        
    private:
        glm::vec2 _velocity;
        glm::vec2 _acceleration;

        bool _useAreaAsMass;
        float _mass;
        float _density;
        float _inverseMass;
        bool _static;

        float _friction;

    public:
        glm::vec2 GetVelocity() const { return _velocity; }
        float GetInverseMass() const { return _inverseMass; }
        bool IsStatic() const { return _static; }
        void ClearVelocity() { _velocity = glm::vec2(0.0f, 0.0f); }
        void SetVelocity(const glm::vec2 velocity) { _velocity = velocity; }
    };

    REGISTER_COMPONENT(Rigidbody)
}
