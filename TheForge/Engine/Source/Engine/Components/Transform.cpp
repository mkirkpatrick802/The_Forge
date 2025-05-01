#include "Transform.h"

#include "imgui.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/ImGuiHelper.h"

Engine::Transform::Transform(): _position(), _localPosition(), _rotation(0), _localRotation(0)
{
    
}

void Engine::Transform::UpdateWorldTransform(const Transform* parent)
{
    if(parent)
    {
        // Update position
        const glm::vec2 rotatedPos = RotateVector(_localPosition, parent->_rotation);
        _position = parent->_position + rotatedPos;
        
        // Update rotation
        _rotation = parent->_rotation + _localRotation;
        
    }
    else
    {
        // If no parent, world = local
        _position = _localPosition;
        _rotation = _localRotation;
    }
}

void Engine::Transform::DrawDetails()
{
    if (ImGuiHelper::InputVector2("Position", _localPosition))
        gameObject->UpdateWorldTransform();

    ImGui::Spacing();
    ImGui::PushItemWidth(50);
    ImGui::Text("Rotation:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    if(ImGui::InputFloat("##Angle", &_localRotation))
        gameObject->UpdateWorldTransform();
    
    ImGui::PopItemWidth();
}

nlohmann::json Engine::Transform::Serialize()
{
    nlohmann::json data =  Component::Serialize();
    data[JsonKeywords::GAMEOBJECT_POSITION_X] = _localPosition.x;
    data[JsonKeywords::GAMEOBJECT_POSITION_Y] = _localPosition.y;
    data[JsonKeywords::GAMEOBJECT_ROTATION] = _localRotation;
    return data;
}

void Engine::Transform::Deserialize(const json& data)
{
    Component::Deserialize(data);

    if (data.contains(JsonKeywords::GAMEOBJECT_POSITION_X) && data.contains(JsonKeywords::GAMEOBJECT_POSITION_Y))
    {
        _localPosition.x = data[JsonKeywords::GAMEOBJECT_POSITION_X];
        _localPosition.y = data[JsonKeywords::GAMEOBJECT_POSITION_Y];
    }

    if (data.contains(JsonKeywords::GAMEOBJECT_ROTATION))
        _localRotation = data[JsonKeywords::GAMEOBJECT_ROTATION];
}

void Engine::Transform::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_position);
    stream.Write(_localPosition);
    stream.Write(_rotation);
    stream.Write(_localRotation);
}

void Engine::Transform::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_position);
    stream.Read(_localPosition);
    stream.Read(_rotation);
    stream.Read(_localRotation);
}
