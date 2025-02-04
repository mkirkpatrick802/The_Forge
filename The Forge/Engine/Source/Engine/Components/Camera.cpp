#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ComponentRegistry.h"
#include "Engine/GameObject.h"
#include "Engine/Rendering/BufferRegistry.h"
#include "Engine/JsonKeywords.h"

Engine::Camera::Camera(): _projection(), _view()
{
    
}

void Engine::Camera::LoadData(const json& data)
{
    
}

nlohmann::json Engine::Camera::SaveData()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<Camera>();
    return data;
}

glm::mat4 Engine::Camera::GetProjectionMatrix()
{
    const auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
    float x_offset = 0;
    float y_offset = 0;
    _projection = glm::ortho(x_offset, sceneFBO->GetSize().x, sceneFBO->GetSize().y, y_offset, -1.0f, 1.0f);
    return _projection;
}

glm::mat4 Engine::Camera::GetViewMatrix()
{
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(gameObject->transform.position.x, gameObject->transform.position.y, 0.0f));
    return _view;
}
