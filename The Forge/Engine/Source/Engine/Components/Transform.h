#pragma once
#include <glm/vec2.hpp>
#include "ComponentUtils.h"

namespace Engine
{
    class Transform final : public Component
    {
    public:
        Transform();

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
        void SetPosition(const glm::vec2& pos) { _position = pos; }
        glm::vec2 GetWorldPosition() const { return _position; }
        glm::vec2 GetLocalPosition() const { return _localPosition; }

        void SetRotation(const float rot) { _rotation = rot; }
        float GetWorldRotation() const {return _rotation;}
        
    };

    REGISTER_COMPONENT(Transform)
}
