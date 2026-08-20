#include "Collider.h"
#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/PolygonCollider.h"
#include "Engine/Components/RectangleCollider.h"
#include <algorithm>
#include <limits>
#include <glm/glm.hpp>

#include "imgui.h"

namespace
{
    // --- separating axis test ---------------------------------------------------
    //
    // Two convex shapes are apart exactly when some axis exists on which their shadows
    // do not overlap; if none does, the axis of *least* overlap is the shortest way out,
    // and that overlap is the penetration depth. Only the depth is reported back:
    // CollisionManager derives the direction from the two centres, as it does for every
    // other shape pair.

    void ProjectPolygon(const Engine::ConvexPolygon& polygon, const glm::vec2 axis, float& outMin, float& outMax)
    {
        outMin = std::numeric_limits<float>::max();
        outMax = std::numeric_limits<float>::lowest();

        for (const glm::vec2 vertex : polygon)
        {
            const float projection = dot(vertex, axis);
            outMin = std::min(outMin, projection);
            outMax = std::max(outMax, projection);
        }
    }

    // Every edge normal of both shapes -- the only axes that can separate two convex
    // polygons, which is what makes the test finite.
    bool ConvexOverlap(const Engine::ConvexPolygon& a, const Engine::ConvexPolygon& b, float& penetration)
    {
        if (a.size() < 3 || b.size() < 3) return false;

        float smallestOverlap = std::numeric_limits<float>::max();

        for (int shape = 0; shape < 2; ++shape)
        {
            const Engine::ConvexPolygon& source = shape == 0 ? a : b;

            for (size_t i = 0; i < source.size(); ++i)
            {
                const glm::vec2 edge = source[(i + 1) % source.size()] - source[i];
                const float length = glm::length(edge);
                if (length <= 1e-6f) continue;

                const glm::vec2 axis = glm::vec2(-edge.y, edge.x) / length;

                float minA, maxA, minB, maxB;
                ProjectPolygon(a, axis, minA, maxA);
                ProjectPolygon(b, axis, minB, maxB);

                const float overlap = std::min(maxA, maxB) - std::max(minA, minB);
                if (overlap <= 0.0f) return false;

                smallestOverlap = std::min(smallestOverlap, overlap);
            }
        }

        penetration = smallestOverlap;
        return true;
    }

    // As above, plus the one axis a circle contributes: from its centre to the nearest
    // vertex. Without it two shapes touching near a corner report an overlap they do not
    // have, because a circle has no edges to derive an axis from.
    bool ConvexCircleOverlap(const Engine::ConvexPolygon& polygon, const glm::vec2 center, const float radius,
                             float& penetration)
    {
        if (polygon.size() < 3 || radius <= 0.0f) return false;

        float smallestOverlap = std::numeric_limits<float>::max();

        glm::vec2 nearestVertex = polygon[0];
        float nearestDistanceSquared = std::numeric_limits<float>::max();
        for (const glm::vec2 vertex : polygon)
        {
            const float distanceSquared = dot(vertex - center, vertex - center);
            if (distanceSquared < nearestDistanceSquared)
            {
                nearestDistanceSquared = distanceSquared;
                nearestVertex = vertex;
            }
        }

        std::vector<glm::vec2> axes;
        axes.reserve(polygon.size() + 1);

        for (size_t i = 0; i < polygon.size(); ++i)
        {
            const glm::vec2 edge = polygon[(i + 1) % polygon.size()] - polygon[i];
            if (const float length = glm::length(edge); length > 1e-6f)
                axes.push_back(glm::vec2(-edge.y, edge.x) / length);
        }

        if (const float nearestDistance = std::sqrt(nearestDistanceSquared); nearestDistance > 1e-6f)
            axes.push_back((nearestVertex - center) / nearestDistance);

        for (const glm::vec2 axis : axes)
        {
            float minPolygon, maxPolygon;
            ProjectPolygon(polygon, axis, minPolygon, maxPolygon);

            const float projectedCenter = dot(center, axis);
            const float minCircle = projectedCenter - radius;
            const float maxCircle = projectedCenter + radius;

            const float overlap = std::min(maxPolygon, maxCircle) - std::max(minPolygon, minCircle);
            if (overlap <= 0.0f) return false;

            smallestOverlap = std::min(smallestOverlap, overlap);
        }

        penetration = smallestOverlap;
        return true;
    }

    Engine::ConvexPolygon RectangleAsPolygon(const Engine::RectangleCollider* rectangle)
    {
        const glm::vec2 center = rectangle->GetCenter();
        const glm::vec2 half = rectangle->GetSize() * 0.5f;

        // Counter-clockwise, and axis-aligned: rectangle colliders ignore rotation by
        // design, and turning one into a polygon must not quietly change that.
        return {
            center + glm::vec2(-half.x, -half.y),
            center + glm::vec2( half.x, -half.y),
            center + glm::vec2( half.x,  half.y),
            center + glm::vec2(-half.x,  half.y)
        };
    }
}

void Engine::Collider::OnActivation()
{
    Component::OnActivation();
}

bool Engine::Collider::CheckCollision(const Collider* collider, float& penetration) const
{
    if (!isEnabled) return false;
    if (!collider) return false;

    // Determine the collision response between the two objects
    // If the response is Ignore, return false immediately (no collision should happen)
    if (const ECollisionResponse response = CollisionProfile::ResolveCollision(profile, collider->profile); response == ECollisionResponse::ECR_Ignore)
        return false;
    
    if (type == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(this);
        
        if (collider->type == EColliderType::ECT_Circle)
        {
            auto* other = dynamic_cast<const CircleCollider*>(collider);
            return CheckCircleCollision(circle, other, penetration) && penetration > COLLIDER_SLOP;

        }
        
        if (collider->type == EColliderType::ECT_Rectangle)
        {
            const auto* other = dynamic_cast<const RectangleCollider*>(collider);
            return CheckCircleRectangleCollision(circle, other, penetration) && penetration > COLLIDER_SLOP;
        }
    }
    else if (type == EColliderType::ECT_Rectangle)
    {
        const auto* rectangle = dynamic_cast<const RectangleCollider*>(this);
        
        if (collider->type == EColliderType::ECT_Circle)
        {
            const auto* circle = dynamic_cast<const CircleCollider*>(collider);
            return CheckCircleRectangleCollision(circle, rectangle, penetration) && penetration > COLLIDER_SLOP;
        }

        if (collider->type == EColliderType::ECT_Rectangle)
        {
            auto* other = dynamic_cast<const RectangleCollider*>(collider);
            return CheckRectangleCollision(rectangle, other, penetration) && penetration > COLLIDER_SLOP;
        }

        if (collider->type == EColliderType::ECT_Polygon)
        {
            const auto* other = dynamic_cast<const PolygonCollider*>(collider);
            return CheckPolygonRectangleCollision(other, rectangle, penetration) && penetration > COLLIDER_SLOP;
        }
    }
    else if (type == EColliderType::ECT_Polygon)
    {
        const auto* polygon = dynamic_cast<const PolygonCollider*>(this);

        if (collider->type == EColliderType::ECT_Circle)
        {
            const auto* circle = dynamic_cast<const CircleCollider*>(collider);
            return CheckPolygonCircleCollision(polygon, circle, penetration) && penetration > COLLIDER_SLOP;
        }

        if (collider->type == EColliderType::ECT_Rectangle)
        {
            const auto* rectangle = dynamic_cast<const RectangleCollider*>(collider);
            return CheckPolygonRectangleCollision(polygon, rectangle, penetration) && penetration > COLLIDER_SLOP;
        }

        if (collider->type == EColliderType::ECT_Polygon)
        {
            const auto* other = dynamic_cast<const PolygonCollider*>(collider);
            return CheckPolygonCollision(polygon, other, penetration) && penetration > COLLIDER_SLOP;
        }
    }

    return false;
}

bool Engine::Collider::CheckPolygonCollision(const PolygonCollider* polygon, const PolygonCollider* other,
                                             float& penetration)
{
    if (polygon == nullptr || other == nullptr) return false;

    penetration = 0.0f;
    bool hit = false;

    // Deepest wins. Two pieces of the same outline can both touch the same thing, and
    // clearing only the shallower of them leaves the object still inside the other.
    for (const ConvexPolygon& piece : polygon->GetWorldPieces())
    {
        for (const ConvexPolygon& otherPiece : other->GetWorldPieces())
        {
            float depth;
            if (ConvexOverlap(piece, otherPiece, depth) && depth > penetration)
            {
                penetration = depth;
                hit = true;
            }
        }
    }

    return hit;
}

bool Engine::Collider::CheckPolygonCircleCollision(const PolygonCollider* polygon, const CircleCollider* circle,
                                                   float& penetration)
{
    if (polygon == nullptr || circle == nullptr) return false;

    const glm::vec2 center = circle->GetCenter();
    const float radius = circle->GetRadius();

    penetration = 0.0f;
    bool hit = false;

    for (const ConvexPolygon& piece : polygon->GetWorldPieces())
    {
        float depth;
        if (ConvexCircleOverlap(piece, center, radius, depth) && depth > penetration)
        {
            penetration = depth;
            hit = true;
        }
    }

    return hit;
}

bool Engine::Collider::CheckPolygonRectangleCollision(const PolygonCollider* polygon, const RectangleCollider* rectangle,
                                                      float& penetration)
{
    if (polygon == nullptr || rectangle == nullptr) return false;

    const ConvexPolygon box = RectangleAsPolygon(rectangle);

    penetration = 0.0f;
    bool hit = false;

    for (const ConvexPolygon& piece : polygon->GetWorldPieces())
    {
        float depth;
        if (ConvexOverlap(piece, box, depth) && depth > penetration)
        {
            penetration = depth;
            hit = true;
        }
    }

    return hit;
}

glm::vec2 Engine::Collider::GetCenter() const
{
    if (gameObject == nullptr) return offset;

    return gameObject->GetWorldPosition() + offset;
}

bool Engine::Collider::CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other,
                                            float& penetration) const
{
    const auto otherPos = other->GetCenter();

    glm::vec2 distance = circle->GetCenter() - otherPos;
    float distanceSquared = dot(distance, distance);
    float radiusSum = circle->GetRadius() + other->GetRadius();
    float radiusSumSquared = radiusSum * radiusSum;

    if (distanceSquared <= radiusSumSquared)
    {
        penetration = radiusSum - std::sqrt(distanceSquared);
        return true;
    }
    
    return false;
}

bool Engine::Collider::CheckCircleRectangleCollision(const CircleCollider* circle,
    const RectangleCollider* rectangle, float& penetration) const
{
    const auto pos = circle->GetCenter();          // Circle center
    const auto otherPos = rectangle->GetCenter(); // Rectangle center
    const auto& halfSize = rectangle->GetSize() * 0.5f; // Half-size

    // Compute rectangle bounds
    const float rectMinX = otherPos.x - halfSize.x;
    const float rectMaxX = otherPos.x + halfSize.x;
    const float rectMinY = otherPos.y - halfSize.y;
    const float rectMaxY = otherPos.y + halfSize.y;

    // Find the closest point to the circle within the rectangle
    const float closestX = std::clamp(pos.x, rectMinX, rectMaxX);
    const float closestY = std::clamp(pos.y, rectMinY, rectMaxY);

    // Vector from closest point to circle center
    const float dx = pos.x - closestX;
    const float dy = pos.y - closestY;
    const float distanceSquared = dx * dx + dy * dy;

    // Check collision and compute penetration depth
    if (const float radius = circle->GetRadius(); distanceSquared <= radius * radius)
    {
        const float distance = sqrt(distanceSquared);
        penetration = radius - distance; // Amount of overlap
        return true;
    }

    return false;
}


bool Engine::Collider::CheckRectangleCollision(const RectangleCollider* rectangle, const RectangleCollider* other,
    float& penetration) const
{
    const auto otherPos = other->GetCenter();
    const auto& otherHalfSize = other->GetSize() * .5f;
    const auto pos = rectangle->GetCenter();
    const auto& halfSize = rectangle->GetSize() * .5f;

    // Compute min/max bounds for each object
    float minA_X = pos.x - halfSize.x;
    float maxA_X = pos.x + halfSize.x;
    float minB_X = otherPos.x - otherHalfSize.x;
    float maxB_X = otherPos.x + otherHalfSize.x;

    float minA_Y = pos.y - halfSize.y;
    float maxA_Y = pos.y + halfSize.y;
    float minB_Y = otherPos.y - otherHalfSize.y;
    float maxB_Y = otherPos.y + otherHalfSize.y;

    // Compute overlap
    float overlapX = std::max(0.0f, std::min(maxA_X, maxB_X) - std::max(minA_X, minB_X));
    float overlapY = std::max(0.0f, std::min(maxA_Y, maxB_Y) - std::max(minA_Y, minB_Y));

    // AABB collision check
    if (overlapX > 0 && overlapY > 0)
    {
        // Penetration is the smallest overlap (to resolve the collision in the most efficient axis)
        penetration = std::min(overlapX, overlapY);
        return true;
    }

    penetration = 0.0f;
    return false;
}

bool Engine::Collider::CheckCollision(const glm::vec2 pos) const
{
    if (!isEnabled) return false;
    
    if (type == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(this);
        const glm::vec2 centre = GetCenter();
        float dx = centre.x - pos.x;
        float dy = centre.y - pos.y;
        float distanceSquared = dx * dx + dy * dy;

        return distanceSquared <= (circle->GetRadius() * circle->GetRadius());
    }
    
    if (type == EColliderType::ECT_Rectangle)
    {
        const auto* rectangle = dynamic_cast<const RectangleCollider*>(this);
        const auto rectPos = GetCenter(); // Center of rectangle
        const auto& halfSize = rectangle->GetSize() * 0.5f;   // Half-size

        // Compute rectangle bounds
        const float rectMinX = rectPos.x - halfSize.x;
        const float rectMaxX = rectPos.x + halfSize.x;
        const float rectMinY = rectPos.y - halfSize.y;
        const float rectMaxY = rectPos.y + halfSize.y;

        // Check if the position is within the bounds of the rectangle
        return (pos.x >= rectMinX && pos.x <= rectMaxX &&
                pos.y >= rectMinY && pos.y <= rectMaxY);
    }

    if (type == EColliderType::ECT_Polygon)
    {
        const auto* polygon = dynamic_cast<const PolygonCollider*>(this);
        if (polygon == nullptr) return false;

        // Inside any one convex piece is inside the shape. The pieces tile the outline
        // without gaps, so a point on a seam is inside both, which is still inside.
        for (const ConvexPolygon& piece : polygon->GetWorldPieces())
        {
            bool insidePiece = true;
            for (size_t i = 0; i < piece.size() && insidePiece; ++i)
            {
                const glm::vec2 edge = piece[(i + 1) % piece.size()] - piece[i];
                insidePiece = edge.x * (pos.y - piece[i].y) - edge.y * (pos.x - piece[i].x) >= 0.0f;
            }

            if (insidePiece) return true;
        }

        return false;
    }

    return false;
}

void Engine::Collider::SetCollisionResponseByObject(const ECollisionObjectType object, const ECollisionResponse response)
{
    profile.SetResponse(object, response);
}

void Engine::Collider::SetCollisionResponseToAllObjects(const ECollisionResponse response)
{
    for (const auto key : profile.responseMap | std::views::keys)
    {
        profile.SetResponse(key, response);
    }
}

void Engine::Collider::DrawDetails()
{
    ImGui::Spacing();
    ImGui::PushItemWidth(150);

    // Where the shape sits relative to the object's origin.
    float offsetValues[2] = { offset.x, offset.y };
    if (ImGui::DragFloat2("Offset", offsetValues, 0.5f))
        offset = glm::vec2(offsetValues[0], offsetValues[1]);

    ImGui::Spacing();
    ImGui::Text("Collision Profile Settings");

    if (ImGui::BeginCombo("Collision Type", ToString(profile.type).c_str()))
    {
        // Enumerated from the real values. This used to count from 0 to
        // (int)ECOT_Max -- which is -1, because the sentinel is 0xFFFFFFFF -- so the
        // loop never ran once and the list came up empty.
        for (const ECollisionObjectType otype : AllCollisionObjectTypes())
        {
            const bool isSelected = (profile.type == otype);

            if (ImGui::Selectable(ToString(otype).c_str(), isSelected))
                profile.type = otype;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    const char* collisionResponseNames[] = { "Ignore", "Overlap", "Block" };
    for (auto& [fst, snd] : profile.responseMap)
    {
        const ECollisionObjectType otherType = fst;
        ECollisionResponse& response = snd;

        std::string label = "Response to " + ToString(otherType); // Create a unique label

        int responseIndex = static_cast<int>(response);
        if (ImGui::Combo(label.c_str(), &responseIndex, collisionResponseNames, IM_ARRAYSIZE(collisionResponseNames)))
        {
            response = static_cast<ECollisionResponse>(responseIndex); // Update response map
        }
    }
    ImGui::PopItemWidth();
}

nlohmann::json Engine::Collider::Serialize()
{
    nlohmann::json data =  Component::Serialize();
    data["collisionProfile"] = profile;
    data["Offset X"] = offset.x;
    data["Offset Y"] = offset.y;
    return data;
}

void Engine::Collider::Deserialize(const json& data)
{
    Component::Deserialize(data);
    if (data.contains("collisionProfile"))
        data.at("collisionProfile").get_to(profile);

    if (data.contains("Offset X")) offset.x = data["Offset X"];
    if (data.contains("Offset Y")) offset.y = data["Offset Y"];
}

void Engine::Collider::Write(NetCode::OutputByteStream& stream) const
{
    Component::Write(stream);

    stream.Write(type);
    profile.Write(stream);
    stream.Write(isEnabled);
    stream.Write(offset);
}

void Engine::Collider::Read(NetCode::InputByteStream& stream)
{
    Component::Read(stream);

    stream.Read(type);
    profile.Read(stream);
    stream.Read(isEnabled);
    stream.Read(offset);
}