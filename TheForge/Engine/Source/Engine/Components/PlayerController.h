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

        // Input replication. A client serializes its input in WriteInput; the host
        // applies it through ReadInput, simulates the result, and replicates the
        // resulting state back out as part of the normal world state update.
        // Default to no-ops so controllers that take no input still work.
        virtual void WriteInput(NetCode::OutputByteStream& stream) const {}
        virtual void ReadInput(NetCode::InputByteStream& stream) {}

        bool IsLocalPlayer() const;

    private:
        uint64_t _controllingPlayer = 0;

    public:
        uint64_t GetControllingPlayerID() const {return _controllingPlayer;}
    };

    REGISTER_COMPONENT(PlayerController);
}
