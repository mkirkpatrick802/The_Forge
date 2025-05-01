#pragma once
#include "Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class CircleCollider final : public Collider
    {
    public:
        CircleCollider();
        void DrawDetails() override;
        
        nlohmann::json Serialize() override;
        void Deserialize(const json& data) override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;

    private:
        float _radius;

    public:
        float GetRadius() const { return _radius; }
    };

    REGISTER_COMPONENT(CircleCollider)
}
