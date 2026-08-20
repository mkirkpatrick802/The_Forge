#include "EditorCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/EventSystem.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/Rendering/BufferRegistry.h"
#include "Engine/System.h"

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
        ZoomOut(_zoomSpeed);
    else if(delta < 0)
        ZoomIn(_zoomSpeed);
}

void Editor::EditorCamera::UpdatePosition(const glm::vec2 delta)
{
    float scaledDragSpeed = _dragSpeed / _zoom;
    _position += delta * scaledDragSpeed;
}

void Editor::EditorCamera::FocusOn(const glm::vec2 worldPosition)
{
    // _position translates the world past a fixed viewpoint rather than moving a camera
    // through it, so it is the negation of where the view is centred -- and Y is negated
    // again on top of that, because world +Y is up while render +Y is down.
    _position = glm::vec2(-worldPosition.x, worldPosition.y);
}

glm::mat4 Editor::EditorCamera::GetProjectionMatrix()
{
    const auto sceneFBO = Engine::BufferRegistry::GetRegistry()->GetBuffer(Engine::BufferRegistry::BufferType::SCENE);
    float width = sceneFBO->GetSize().x / _zoom;  // Apply zoom to width
    float height = sceneFBO->GetSize().y / _zoom; // Apply zoom to height

    // Centred on half the app window, not on zero.
    //
    // Every renderable offsets itself by AppWindowSize/2 on the way in, via
    // CameraManager::ConvertWorldToScreen -- so that is where world origin actually
    // lands in render space. The gameplay Camera accounts for it (it centres its ortho
    // on windowSize/2); this one used to centre on 0, which put the whole world about
    // half a window off to one side and left the editor looking at empty space until
    // you panned a long way.
    const glm::vec2 window = Engine::GetAppWindowSize();
    const float centreX = window.x * 0.5f;
    const float centreY = window.y * 0.5f;

    // Y is intentionally inverted (bottom > top): render space is Y-down.
    _projection = glm::ortho(centreX - width / 2.0f, centreX + width / 2.0f,
                             centreY + height / 2.0f, centreY - height / 2.0f,
                             -1.0f, 1.0f);
    return _projection;
}

glm::vec2 Editor::EditorCamera::ViewportToWorld(const glm::vec2 viewportPixels)
{
    const auto sceneFBO = Engine::BufferRegistry::GetRegistry()->GetBuffer(Engine::BufferRegistry::BufferType::SCENE);
    const glm::vec2 imageSize = sceneFBO->GetSize();
    if (imageSize.x <= 0.0f || imageSize.y <= 0.0f) return glm::vec2(0.0f);

    // Image pixel -> NDC. The scene texture is drawn with its V flipped (uv0 = (0,1)),
    // so the top of the image is NDC +1.
    const glm::vec2 ndc((viewportPixels.x / imageSize.x) * 2.0f - 1.0f,
                        1.0f - (viewportPixels.y / imageSize.y) * 2.0f);

    const glm::mat4 inverseViewProjection = glm::inverse(GetProjectionMatrix() * GetViewMatrix());
    const glm::vec4 render = inverseViewProjection * glm::vec4(ndc, 0.0f, 1.0f);

    // Undo what every renderable applies on the way in via ConvertWorldToScreen: an
    // offset of half the *app window* -- not half the viewport -- and a Y flip, because
    // world +Y is up while render +Y is down.
    const glm::vec2 windowSize = Engine::GetAppWindowSize();
    return { render.x - windowSize.x * 0.5f, windowSize.y * 0.5f - render.y };
}

glm::mat4 Editor::EditorCamera::GetViewMatrix()
{
    _view = glm::translate(glm::mat4(1.0f), glm::vec3(_position.x, _position.y, 0.0f));
    return _view;
}
