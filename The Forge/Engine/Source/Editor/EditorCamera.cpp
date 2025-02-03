#include "EditorCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/EventSystem.h"
#include "Engine/Rendering/BufferRegistry.h"

std::shared_ptr<Editor::EditorCamera> Editor::EditorCamera::_instance = nullptr;

std::shared_ptr<Editor::EditorCamera> Editor::EditorCamera::GetInstance()
{
    _instance = _instance == nullptr ? std::make_shared<EditorCamera>() : _instance;
    return _instance;
}

Editor::EditorCamera::EditorCamera(): _position(0, 0), _projection(), _view()
{

}

void Editor::EditorCamera::CleanUp()
{
    _instance.reset();
}

void Editor::EditorCamera::UpdatePosition(const glm::vec2 delta)
{
    _position += delta * _dragSpeed;
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
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(_position.x, _position.y, 0.0f));
    return _view;
}
