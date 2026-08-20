#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "json.hpp"

namespace Engine
{
    // A closed convex loop, counter-clockwise, in sprite-local units: the origin is the
    // sprite's centre and +Y is up -- the frame a SpriteRenderer draws in, and the one
    // Collider::GetCenter works in. One unit is one pixel.
    using ConvexPolygon = std::vector<glm::vec2>;

    // A sprite's solid area, as convex pieces.
    //
    // Convex, plural, because collision needs it that way: a traced sprite is almost
    // always concave -- a hull plate with a docking notch, a ring with a hole through
    // it -- and the separating axis test only means anything on convex shapes. The
    // decomposition happens once at import; at runtime this is just a list of loops to
    // test in turn.
    struct SpriteOutline
    {
        std::vector<ConvexPolygon> pieces;

        bool IsEmpty() const { return pieces.empty(); }

        nlohmann::json Serialize() const;
        static SpriteOutline Deserialize(const nlohmann::json& data);
    };

    struct OutlineSettings
    {
        // A pixel is solid at or above this alpha. Feathered or anti-aliased edges are
        // why this is not simply "> 0": tracing every barely-visible pixel produces an
        // outline a pixel or two larger than the sprite looks, all the way round.
        uint8_t alphaThreshold = 128;

        // How far, in pixels, the simplified outline may stray from the traced one.
        //
        // The raw trace follows pixel edges, so it is a staircase: a diagonal edge costs
        // two vertices per pixel. Collision does not need that, and every vertex is an
        // axis to test. Around 1.5 keeps the silhouette while cutting a 64x64 sprite's
        // outline from hundreds of vertices to tens.
        float simplifyTolerance = 1.5f;

        // Beyond this many convex pieces the decomposition is thrown away and the convex
        // hull used instead. A noisy or heavily feathered sprite can otherwise produce
        // hundreds of slivers, each costing a separating axis test every frame -- at
        // which point a per-pixel collider is slower than the sprite is precise.
        int maxPieces = 32;

        // Solid regions smaller than this are dropped, in pixels squared. Stray pixels
        // and anti-aliasing crumbs are not collision geometry.
        float minimumArea = 4.0f;
    };

    // Traces an image's alpha channel and decomposes the result into convex pieces.
    //
    // Decodes the image, so this is import-time work -- the result belongs in the
    // asset's sidecar, which is what AssetMetadata does with it. Returns an empty
    // outline if the image cannot be read or has no solid pixels.
    SpriteOutline BuildSpriteOutline(const std::string& imagePath, const OutlineSettings& settings = {});

    // The convex decomposition of one simple polygon, exposed for testing and for
    // callers that already have an outline. `loop` may be concave; it must not
    // self-intersect. Winding is normalised internally.
    std::vector<ConvexPolygon> DecomposeToConvex(const ConvexPolygon& loop, int maxPieces);

    // Positive for a counter-clockwise loop.
    float SignedArea(const ConvexPolygon& polygon);
}
