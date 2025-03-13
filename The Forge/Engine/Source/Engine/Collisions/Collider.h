#pragma once
#include "CollisionProfile.h"
#include "Engine/Components/Component.h"
#include "Engine/DelegateSystem.h"

class Engine::GameObject;

DECLARE_DELEGATE_ONE_PARAM(OnOverlapBeginDelegate, Engine::GameObject*, overlappedObject)
DECLARE_DELEGATE_ONE_PARAM(OnOverlapDelegate, Engine::GameObject*, overlappingObject)
DECLARE_DELEGATE_ONE_PARAM(OnOverlapEndDelegate, Engine::GameObject*, overlappedObject)

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

    constexpr float COLLIDER_SLOP = 0.01f;
    
    class Collider : public Component
    {
    public:
        void OnActivation() override;
        ~Collider() override = default;

        void DrawDetails() override;
        nlohmann::json Serialize() override;
        void Deserialize(const json& data) override;
        
        bool CheckCollision(const Collider* collider, float& penetration) const;
        bool CheckCollision(glm::vec2 pos) const;

    private:
        bool CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other, float& penetration) const;
        bool CheckCircleRectangleCollision(const CircleCollider* circle, const RectangleCollider* rectangle, float& penetration) const;
        bool CheckRectangleCollision(const RectangleCollider* rectangle, const RectangleCollider* other, float& penetration) const;

    public:
        OnOverlapBeginDelegate OnOverlapBegin;
        OnOverlapDelegate OnOverlap;
        OnOverlapEndDelegate OnOverlapEnd;
        
    protected:
        EColliderType type = EColliderType::ECT_None;
        CollisionProfile profile;

    private:
        bool isEnabled = true;

    public:
        EColliderType GetType() const { return type;}
        CollisionProfile GetCollisionProfile() const { return profile;}
        void SetEnabled(const bool enabled) { isEnabled = enabled; }
        bool GetEnabled() const { return isEnabled; }
    };
}
