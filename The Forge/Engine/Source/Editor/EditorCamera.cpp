#include "EditorCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/EventSystem.h"
#include "Engine/Rendering/BufferRegistry.h"

std::shared_ptr<Editor::EditorCamera> Editor::EditorCamera::_instance = nullptr;

std::shared_ptr<Editor::EditorCamera> Editor::EditorCamera::GetInstance()
{
    return _instance == nullptr ? std::make_shared<EditorCamera>() : _instance;
}

void Editor::EditorCamera::Update(float deltaTime)
{
    
}

glm::mat4 Editor::EditorCamera::GetProjectionMatrix()
{
    const auto sceneFBO = Engine::BufferRegistry::GetRegistry()->GetBuffer(Engine::BufferRegistry::BufferType::SCENE);
    float x_offset = 0;
    float y_offset = 0;
    _projection = glm::ortho(x_offset, sceneFBO->GetSize().x, sceneFBO->GetSize().y, y_offset, -1.0f, 1.0f);
    return _projection;
}

glm::mat4 Editor::EditorCamera::GetViewMatrix()
{
    //_view = lookAt(glm::vec3(_position.x, _position, 10), glm::vec3(_position.x, _position, 10) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return _view;
}
