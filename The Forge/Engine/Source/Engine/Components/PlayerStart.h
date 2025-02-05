#pragma once
#include "Component.h"
#include "ComponentUtils.h"

namespace Engine
{
    class PlayerStart final : public Component
    {
    public:
        PlayerStart() = default;
        
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;
    };

    REGISTER_COMPONENT(PlayerStart);
}
