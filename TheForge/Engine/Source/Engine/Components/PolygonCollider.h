#pragma once
#include "Collider.h"
#include "ComponentUtils.h"
#include "Engine/Collisions/SpriteOutline.h"

namespace Engine
{
    // A collider shaped like the sprite it sits next to, traced from the image's alpha.
    //
    // The shape is not computed here. It is traced and decomposed into convex pieces at
    // import time and stored in the image's sidecar, so this component only reads it --
    // a level load does no image work, and a headless server never decodes a texture it
    // has no use for.
    //
    // Concave sprites are the normal case, which is why the stored shape is a *set* of
    // convex pieces: the separating axis test only means anything on a convex shape, so
    // a concave outline is tested one piece at a time.
    class PolygonCollider final : public Collider
    {
    public:
        PolygonCollider();

        void Start() override;

        void DrawDetails() override;
        nlohmann::json Serialize() override;
        void Deserialize(const json& data) override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;

        // The convex pieces in world space: scaled, rotated by the object and moved to
        // GetCenter(). Rebuilt only when the object has actually moved -- a piece per
        // sprite times a few axes per piece is enough work to be worth not repeating for
        // every candidate the broadphase hands over.
        const std::vector<ConvexPolygon>& GetWorldPieces() const;

        // The world-space bounds of every piece. What the quadtree sorts on.
        void GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax) const;

        bool HasShape() const { EnsureShapeLoaded(); return !_localShape.IsEmpty(); }

        // Re-reads the outline from the sprite's sidecar. Called on Start, and by the
        // inspector button after a re-import.
        //
        // Const because the shape is loaded on demand -- see EnsureShapeLoaded. Nothing
        // about the collider as the rest of the engine sees it changes.
        void ReloadShape() const;

    private:
        // Loads the shape the first time anything asks for it.
        //
        // Start() is not enough on its own: components do not tick in the editor, so a
        // collider that only loaded there would have no shape at all while you were
        // authoring with it -- nothing to draw, nothing to pick, an empty inspector.
        // Deserialization order is no better, since the SpriteRenderer this reads from
        // may not exist yet when this component is built.
        void EnsureShapeLoaded() const;

    private:
        // Which image to take the shape from. Empty means "the sprite on this object",
        // which is what makes the component work by just being added.
        std::string _spritePath;

        // Uniform scale on the traced outline, so a collider can be tightened or padded
        // without re-authoring the art.
        float _scale = 1.0f;

        mutable SpriteOutline _localShape;
        mutable bool _shapeLoaded = false;

        // The transform the cached world pieces were built for.
        mutable std::vector<ConvexPolygon> _worldPieces;
        mutable glm::vec2 _cachedCenter = glm::vec2(0.0f);
        mutable float _cachedRotation = 0.0f;
        mutable float _cachedScale = 0.0f;
        mutable bool _cacheValid = false;

    public:
        float GetScale() const { return _scale; }
        void SetScale(const float scale) { _scale = scale; _cacheValid = false; }

        const std::string& GetSpritePath() const { return _spritePath; }
    };

    REGISTER_COMPONENT(PolygonCollider)
}
