#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

#include "BufferRegistry.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"

Engine::Camera::Camera(): _projection()
{
    RegisterComponent(ComponentID, "Camera");
}

void Engine::Camera::CleanUp()
{
    
}

void Engine::Camera::LoadData(const json& data)
{
    
}

nlohmann::json Engine::Camera::SaveData()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = ComponentID;
    return data;
}

glm::mat4 Engine::Camera::GetProjectionMatrix()
{
    const auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
    float left = -sceneFBO->GetSize().x / 2.f;
    float right = sceneFBO->GetSize().x / 2.f;
    float bottom = sceneFBO->GetSize().y / 2.f;
    float top = -sceneFBO->GetSize().y / 2.f;
    _projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    return _projection;
}