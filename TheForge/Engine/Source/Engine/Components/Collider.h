#pragma once
#include "Engine/Components/Component.h"
#include "Engine/DelegateSystem.h"
#include "Engine/Collisions/CollisionProfile.h"

class Engine::GameObject;

DECLARE_DELEGATE_ONE_PARAM(OnOverlapBeginDelegate, Engine::GameObject*, overlappedObject)
DECLARE_DELEGATE_ONE_PARAM(OnOverlapDelegate, Engine::GameObject*, overlappingObject)
DECLARE_DELEGATE_ONE_PARAM(OnOverlapEndDelegate, Engine::GameObject*, overlappedObject)

namespace Engine
{
    class RectangleCollider;
    class CircleCollider;
    class PolygonCollider;

    enum class EColliderType : uint8_t
    {
        ECT_None = 0,
        ECT_Rectangle,
        ECT_Circle,
        ECT_Polygon
    };

    constexpr float COLLIDER_SLOP = 0.01f;
    
    class Collider : public Component
    {
    public:
        void OnActivation() override;
        ~Collider() override = default;
        
        bool CheckCollision(const Collider* collider, float& penetration) const;
        bool CheckCollision(glm::vec2 pos) const;

        // Where this collider actually sits: the object's world position plus the
        // offset. Everything that needs a collider's centre goes through here, so a
        // shape can be placed away from its object's origin -- a cockpit whose hitbox
        // covers only its lower half, say.
        //
        // The offset is *not* rotated by the object. Collider bounds are axis-aligned
        // and ignore rotation already, so rotating only the offset would place the
        // shape somewhere its own bounds do not follow.
        glm::vec2 GetCenter() const;

        void SetCollisionResponseByObject(ECollisionObjectType object, ECollisionResponse response);
        void SetCollisionResponseToAllObjects(ECollisionResponse response);
        
        void DrawDetails() override;
        nlohmann::json Serialize() override;
        void Deserialize(const json& data) override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;
        
    private:
        bool CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other, float& penetration) const;
        bool CheckCircleRectangleCollision(const CircleCollider* circle, const RectangleCollider* rectangle, float& penetration) const;
        bool CheckRectangleCollision(const RectangleCollider* rectangle, const RectangleCollider* other, float& penetration) const;

        // A traced outline is a set of convex pieces, so each of these tests every piece
        // and reports the deepest hit -- that is the one a resolution has to clear.
        static bool CheckPolygonCollision(const PolygonCollider* polygon, const PolygonCollider* other, float& penetration);
        static bool CheckPolygonCircleCollision(const PolygonCollider* polygon, const CircleCollider* circle, float& penetration);
        static bool CheckPolygonRectangleCollision(const PolygonCollider* polygon, const RectangleCollider* rectangle, float& penetration);

    public:
        OnOverlapBeginDelegate OnOverlapBegin;
        OnOverlapDelegate OnOverlap;
        OnOverlapEndDelegate OnOverlapEnd;
        
    protected:
        EColliderType type = EColliderType::ECT_None;
        CollisionProfile profile;

        // Local to the game object, in world units.
        glm::vec2 offset = glm::vec2(0.0f);

    private:
        bool isEnabled = true;

    public:
        EColliderType GetColliderType() const { return type;}
        CollisionProfile GetCollisionProfile() const { return profile;}
        void SetEnabled(const bool enabled) { isEnabled = enabled; }
        bool GetEnabled() const { return isEnabled; }

        glm::vec2 GetOffset() const { return offset; }
        void SetOffset(const glm::vec2 value) { offset = value; }
    };
}
