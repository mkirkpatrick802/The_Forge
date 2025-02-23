#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    // TODO: Move this to the game source not the engine
    class PlayerController final : public Component
    {
        friend class GameModeBase;
        
    public:
        
        void Update(float deltaTime) override;
        void CollectInput(float deltaTime);
        void UpdatePosition(float deltaTime);

        void Read(NetCode::InputByteStream& stream) override;
        void Write(NetCode::OutputByteStream& stream) const override;

        bool IsLocalPlayer() const;
        
    private:
        uint64_t _controllingPlayer = 0;

        // Movement
        const float _acceleration = 60.0f; // Adjust for responsiveness
        const float _maxSpeed = 100.0f;
        
        glm::vec2 _velocity = glm::vec2(0.0f);

    public:
        uint64_t GetControllingPlayerID() const {return _controllingPlayer;}
    };

    REGISTER_COMPONENT(PlayerController);
}
