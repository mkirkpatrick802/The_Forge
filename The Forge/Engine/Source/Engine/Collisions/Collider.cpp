#include "Collider.h"

#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/RectangleCollider.h"
#include <glm/glm.hpp>

#include "imgui.h"

void Engine::Collider::OnActivation()
{
    Component::OnActivation();
}

void Engine::Collider::DrawDetails()
{
    ImGui::Spacing();
    ImGui::Text("Collision Profile Settings");
    ImGui::PushItemWidth(150);
    if (ImGui::BeginCombo("Collision Type", ToString(profile.type).c_str()))
    {
        for (int i = 0; i < static_cast<int>(ECollisionObjectType::ECOT_Max); i++)
        {
            const auto otype = static_cast<ECollisionObjectType>(i);
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
    return data;
}

void Engine::Collider::Deserialize(const json& data)
{
    Component::Deserialize(data);
    if (data.contains("collisionProfile"))
        data.at("collisionProfile").get_to(profile);
}

bool Engine::Collider::CheckCollision(const Collider* collider, float& penetration) const
{
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
    }

    return false;
}

bool Engine::Collider::CheckCircleCollision(const CircleCollider* circle, const CircleCollider* other,
    float& penetration) const
{
    const auto& otherPos = other->gameObject->GetWorldPosition();

    glm::vec2 distance = circle->gameObject->GetWorldPosition() - otherPos;
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
    const auto& pos = circle->gameObject->GetWorldPosition();  // Circle center
    const auto& otherPos = rectangle->gameObject->GetWorldPosition(); // Rectangle center
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
    const auto& otherPos = other->gameObject->GetWorldPosition();
    const auto& otherSize = other->GetSize();
    const auto& pos = gameObject->GetWorldPosition();
    const auto& size = rectangle->GetSize();

    // AABB (Axis-Aligned Bounding Box) collision check
    return (pos.x < otherPos.x + otherSize.x &&
            pos.x + size.x > otherPos.x &&
            pos.y < otherPos.y + otherSize.y &&
            pos.y + size.y > otherPos.y);
}

bool Engine::Collider::CheckCollision(const glm::vec2 pos) const
{
    if (type == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(this);
        float dx = gameObject->GetWorldPosition().x - pos.x;
        float dy = gameObject->GetWorldPosition().y - pos.y;
        float distanceSquared = dx * dx + dy * dy;

        return distanceSquared <= (circle->GetRadius() * circle->GetRadius());
    }
    
    if (type == EColliderType::ECT_Rectangle)
    {
        const auto* rectangle = dynamic_cast<const RectangleCollider*>(this);
        const auto& rectPos = gameObject->GetWorldPosition(); // Center of rectangle
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


    return false;
}

