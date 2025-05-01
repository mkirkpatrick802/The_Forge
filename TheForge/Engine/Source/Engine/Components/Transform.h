#pragma once
#include <glm/vec2.hpp>
#include "ComponentUtils.h"

namespace Engine
{
    class Transform final : public Component
    {
    public:
        Transform();
        
        void UpdateWorldTransform(const Transform* parent);
        
        void DrawDetails() override;
        nlohmann::json Serialize() override;
        void Deserialize(const json& data) override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;
    
        
    private:
        glm::vec2 _position;            // world position
        glm::vec2 _localPosition;       // local position
        float _rotation;                // world rotation (degrees)
        float _localRotation;           // local rotation (degrees)

    public:
        void SetLocalPosition(const glm::vec2 pos) { _localPosition = pos; }
        glm::vec2 GetWorldPosition() const { return _position; }
        glm::vec2 GetLocalPosition() const { return _localPosition; }

        void SetLocalRotation(const float angle) { _localRotation = angle; }
        float GetWorldRotation() const {return _rotation;}
    };

    REGISTER_COMPONENT(Transform)

    inline glm::vec2 RotateVector(const glm::vec2& v, const float angle)
    {
        const float s = sin(angle);
        const float c = cos(angle);
        return glm::vec2(
            v.x * c - v.y * s,
            v.x * s + v.y * c
        );
    }
}
