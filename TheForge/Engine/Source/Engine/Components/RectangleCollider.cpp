#include "RectangleCollider.h"
#include "SpriteRenderer.h"
#include "Engine/Rendering/ImGuiHelper.h"

Engine::RectangleCollider::RectangleCollider(): _size()
{
    type = EColliderType::ECT_Rectangle;
}

void Engine::RectangleCollider::DrawDetails()
{
    ImGuiHelper::InputVector2("Size", _size, 100);

    Collider::DrawDetails();
}

nlohmann::json Engine::RectangleCollider::Serialize()
{
    nlohmann::json data = Collider::Serialize();
    data["Size X"] = _size.x;
    data["Size Y"] = _size.y;
    return data;
}

void Engine::RectangleCollider::Deserialize(const nlohmann::json& json)
{
    Collider::Deserialize(json);
    if (json.contains("Size X") && json.contains("Size Y"))
    {
        _size.x = json["Size X"];
        _size.y = json["Size Y"];
    }
}

void Engine::RectangleCollider::Write(NetCode::OutputByteStream& stream) const
{
    Collider::Write(stream);

    stream.Write(_size);
}

void Engine::RectangleCollider::Read(NetCode::InputByteStream& stream)
{
    Collider::Read(stream);

    stream.Read(_size);
}
