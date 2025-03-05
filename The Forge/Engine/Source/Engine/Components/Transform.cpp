#include "Transform.h"

#include "imgui.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/ImGuiHelper.h"

Engine::Transform::Transform(): _position(), _localPosition(), _rotation(0), _localRotation(0)
{
}

void Engine::Transform::DrawDetails()
{
    ImGuiHelper::InputVector2("Position", _position);

    ImGui::Spacing();
    ImGui::PushItemWidth(50);
    ImGui::Text("Rotation:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    ImGui::InputFloat("##Angle", &_rotation);
    ImGui::PopItemWidth();
}

nlohmann::json Engine::Transform::Serialize()
{
    nlohmann::json data =  Component::Serialize();
    data[JsonKeywords::GAMEOBJECT_POSITION_X] = _position.x;
    data[JsonKeywords::GAMEOBJECT_POSITION_Y] = _position.y;
    data[JsonKeywords::GAMEOBJECT_ROTATION] = _rotation;
    return data;
}

void Engine::Transform::Deserialize(const json& data)
{
    Component::Deserialize(data);

    if (data.contains(JsonKeywords::GAMEOBJECT_POSITION_X) && data.contains(JsonKeywords::GAMEOBJECT_POSITION_Y))
    {
        _position.x = data[JsonKeywords::GAMEOBJECT_POSITION_X];
        _position.y = data[JsonKeywords::GAMEOBJECT_POSITION_Y];
    }

    if (data.contains(JsonKeywords::GAMEOBJECT_ROTATION))
        _rotation = data[JsonKeywords::GAMEOBJECT_ROTATION];
}

void Engine::Transform::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_position);
    stream.Write(_rotation);
}

void Engine::Transform::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_position);
    stream.Read(_rotation);
}
