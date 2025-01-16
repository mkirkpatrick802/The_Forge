#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

#include "Engine/System.h"

Engine::Camera::Camera(): _projection()
{
}

glm::mat4 Engine::Camera::GetProjectionMatrix()
{
    _projection = glm::ortho(0.0f, 1280.f, 720.f, 0.0f, -1.0f, 1.0f);
    return _projection;
}

Vector2D Engine::Camera::ConvertWorldToScreen(const Vector2D worldPos)
{
    // TODO: Use scene size instead of window size
    const auto screenLocation = Vector2D(worldPos.x + System::GetWindowSize().x / 2, worldPos.y + System::GetWindowSize().y / 2);
    return screenLocation;
}
