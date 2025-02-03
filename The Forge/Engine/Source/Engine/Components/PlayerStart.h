#pragma once
#include "Component.h"
#include "ComponentRegistry.h"

namespace Engine
{
    class PlayerStart final : public Component
    {
    public:
        void LoadData(const json& data) override;
        nlohmann::json SaveData() override;
    };

    REGISTER_COMPONENT(PlayerStart);
}
