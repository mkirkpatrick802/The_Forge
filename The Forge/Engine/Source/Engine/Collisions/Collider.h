#pragma once
#include "../Components/Component.h"
#include "Engine/Components/ComponentUtils.h"

namespace Engine
{
    enum class EColliderType : uint8_t
    {
        ECT_None = 0,
        ECT_Rectangle,
        ECT_Circle
    };
    
    class Collider : public Component
    {
    public:
        void OnActivation() override;
        ~Collider() override = default;
        virtual bool CheckCollision(const Collider* collider) const;
        virtual bool CheckCollision(glm::vec2 pos) const;
        
    protected:
        EColliderType type = EColliderType::ECT_None;

    public:
        EColliderType GetType() const { return type;}
        
    };
}
