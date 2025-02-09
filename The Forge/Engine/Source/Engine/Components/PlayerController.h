#pragma once
#include "Component.h"
#include "ComponentUtils.h"

namespace Engine
{
    class PlayerController final : public Component
    {
    public:
        
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;

        void Update(float deltaTime) override;

    private:
        float _walkSpeed = 100;
    };

    REGISTER_COMPONENT(PlayerController);
}
