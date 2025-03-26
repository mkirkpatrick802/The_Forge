#include "EditorCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/EventSystem.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
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

void Editor::EditorCamera::Update()
{
    if(const int32_t delta = Engine::GetInputManager().GetMouseWheelDelta(); delta > 0)
        ZoomIn(_zoomSpeed);
    else if(delta < 0)
        ZoomOut(_zoomSpeed);
}

void Editor::EditorCamera::UpdatePosition(const glm::vec2 delta)
{
    _position += delta * _dragSpeed;
}

glm::mat4 Editor::EditorCamera::GetProjectionMatrix()
{
    const auto sceneFBO = Engine::BufferRegistry::GetRegistry()->GetBuffer(Engine::BufferRegistry::BufferType::SCENE);
    float width = sceneFBO->GetSize().x / _zoom;  // Apply zoom to width
    float height = sceneFBO->GetSize().y / _zoom; // Apply zoom to height

    float x_offset = -width / 2.0f;  // Centering logic
    float y_offset = -height / 2.0f;
    
    _projection = glm::ortho(x_offset, width + x_offset, height + y_offset, y_offset, -1.0f, 1.0f);
    return _projection;
}

glm::mat4 Editor::EditorCamera::GetViewMatrix()
{
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(_position.x, _position.y, 0.0f));
    return _view;
}
