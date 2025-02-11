#pragma once
#include "Component.h"
#include "ComponentUtils.h"

namespace Engine
{
    class PlayerController final : public Component
    {
        friend class GameModeBase;
        
    public:
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;

        void Update(float deltaTime) override;

        void Read(NetCode::InputByteStream& stream) override;
        void Write(NetCode::OutputByteStream& stream) const override;
        
    private:
        float _walkSpeed = 100;
        uint64_t _controllingPlayer = 0; // TODO: this needs to be set somewheres
    };

    REGISTER_COMPONENT(PlayerController);
}
