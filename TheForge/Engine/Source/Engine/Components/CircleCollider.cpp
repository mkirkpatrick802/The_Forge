#include "CircleCollider.h"

#include "Collider.h"
#include "imgui.h"
#include "RectangleCollider.h"

Engine::CircleCollider::CircleCollider(): _radius(32)
{
    type = EColliderType::ECT_Circle;
}

void Engine::CircleCollider::DrawDetails()
{
    ImGui::InputFloat("Radius", &_radius);

    Collider::DrawDetails();
}

nlohmann::json Engine::CircleCollider::Serialize()
{
    nlohmann::json data = Collider::Serialize();
    data["radius"] = _radius;
    return data;
}

void Engine::CircleCollider::Deserialize(const json& data)
{
    Collider::Deserialize(data);
    if (data.contains("radius"))
        _radius = data["radius"];
}

void Engine::CircleCollider::Write(NetCode::OutputByteStream& stream) const
{
    Collider::Write(stream);

    stream.Write(_radius);
}

void Engine::CircleCollider::Read(NetCode::InputByteStream& stream)
{
    Collider::Read(stream);

    stream.Read(_radius);
}

