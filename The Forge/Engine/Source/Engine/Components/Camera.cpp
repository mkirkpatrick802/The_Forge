#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ComponentRegistry.h"
#include "Engine/GameObject.h"
#include "Engine/Rendering/BufferRegistry.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraManager.h"

Engine::Camera::Camera(): _projection(), _view()
{
    
}

void Engine::Camera::OnActivation()
{
    
}

glm::mat4 Engine::Camera::GetProjectionMatrix()
{
    // FBO size is your window size
    auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
    glm::vec2 windowSize = sceneFBO->GetSize();

    // Set your camera bounds
    _bounds.left = 0; 
    _bounds.right = windowSize.x;
    _bounds.bottom = windowSize.y;
    _bounds.top = 0;

    // Create the orthographic projection matrix with camera bounds
    _projection = glm::ortho(_bounds.left, _bounds.right, _bounds.bottom, _bounds.top, -1.0f, 1.0f);

    // Return the projection matrix
    return _projection;
}

glm::mat4 Engine::Camera::GetViewMatrix()
{
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(-gameObject->GetWorldPosition().x, gameObject->GetWorldPosition().y, 0.0f));
    return _view;
}
