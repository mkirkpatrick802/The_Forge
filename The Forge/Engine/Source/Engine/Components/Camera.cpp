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
    // Get window size from the scene FBO
    auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
    glm::vec2 windowSize = sceneFBO->GetSize();

    // Compute half-size for correct centering
    float halfWidth = (windowSize.x * 0.5f) / _zoom;
    float halfHeight = (windowSize.y * 0.5f) / _zoom;

    // Maintain the camera centered while zooming
    float centerX = windowSize.x * 0.5f;
    float centerY = windowSize.y * 0.5f;

    // Update bounds
    _bounds.left = centerX - halfWidth;
    _bounds.right = centerX + halfWidth;
    _bounds.bottom = centerY + halfHeight;
    _bounds.top = centerY - halfHeight;

    // Apply orthographic projection
    _projection = glm::ortho(_bounds.left, _bounds.right, _bounds.bottom, _bounds.top, -1.0f, 1.0f);

    return _projection;
}



glm::mat4 Engine::Camera::GetViewMatrix()
{
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(-gameObject->GetWorldPosition().x, gameObject->GetWorldPosition().y, 0.0f));
    return _view;
}
