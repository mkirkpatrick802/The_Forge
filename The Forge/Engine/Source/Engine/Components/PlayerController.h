#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    class PlayerController : public Component
    {
        friend class GameModeBase;
        
    public:

        void Start() override;
        void Update(float deltaTime) override;

        void Read(NetCode::InputByteStream& stream) override;
        void Write(NetCode::OutputByteStream& stream) const override;

        bool IsLocalPlayer() const;

    private:
        uint64_t _controllingPlayer = 0;

    public:
        uint64_t GetControllingPlayerID() const {return _controllingPlayer;}
    };

    REGISTER_COMPONENT(PlayerController);
}
