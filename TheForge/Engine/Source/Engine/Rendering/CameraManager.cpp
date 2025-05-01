#include "CameraManager.h"
#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"
#include "Engine/EventSystem.h"
#include "Engine/System.h"

Engine::CameraManager& Engine::CameraManager::GetInstance()
{
    static auto instance = std::make_unique<CameraManager>();
    return *instance;
}

Engine::CameraManager::CameraManager()
{
    EventSystem::GetInstance()->RegisterEvent("Editor Enabled", this, &CameraManager::ResetActiveCamera);
}

Engine::CameraManager::~CameraManager()
{
    EventSystem::GetInstance()->DeregisterEvent("Editor Enabled", this);
}

glm::vec2 Engine::CameraManager::ConvertWorldToScreen(const glm::vec2 worldPos) const
{
    const auto screenLocation = glm::vec2(worldPos.x + GetAppWindowSize().x / 2, (worldPos.y + GetAppWindowSize().y / 2));
    return screenLocation;
}

glm::vec2 Engine::CameraManager::ConvertScreenToWorld(const glm::vec2 screenPos) const
{
    if (!GetActiveCamera()) return glm::vec2(0.0f);

    const auto& camera = *GetActiveCamera();
    const glm::vec2 windowSize = GetAppWindowSize();

    // Convert screen position relative to the center, apply inverse zoom
    glm::vec2 worldLocation;
    worldLocation.x = (screenPos.x - windowSize.x * 0.5f) / camera.GetZoom();
    worldLocation.y = (windowSize.y * 0.5f - screenPos.y) / camera.GetZoom();

    // Offset by camera world position
    return worldLocation + camera.gameObject->GetWorldPosition();
}

void Engine::CameraManager::ResetActiveCamera(const void* p)
{
    _currentCamera = nullptr;
}
