#pragma once
#include "Collider.h"
#include "ComponentUtils.h"

namespace Engine
{
    class RectangleCollider final : public Collider
    {
    public:
        RectangleCollider();
        
        void DrawDetails() override;
        nlohmann::json Serialize() override;
        void Deserialize(const nlohmann::json& json) override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;
        
    private:
        glm::vec2 _size;

    public:
        glm::vec2 GetSize() const { return _size; }
        
    };

    REGISTER_COMPONENT(RectangleCollider);
}
