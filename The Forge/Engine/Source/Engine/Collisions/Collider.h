#pragma once
#include "../Components/Component.h"
#include "Engine/Components/ComponentUtils.h"

namespace Engine
{
    class RectangleCollider;
    class CircleCollider;

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
        
        bool CheckCollision(const Collider* collider) const;
        bool CheckCollision(glm::vec2 pos) const;

    private:
        bool CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other) const;
        bool CheckCircleRectangleCollision(const CircleCollider* circle, const RectangleCollider* rectangle) const;
        bool CheckRectangleCollision(const RectangleCollider* rectangle, const RectangleCollider* other) const;
        
    protected:
        EColliderType type = EColliderType::ECT_None;

    public:
        EColliderType GetType() const { return type;}
        
    };
}
