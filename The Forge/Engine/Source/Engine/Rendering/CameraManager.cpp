#include "CameraManager.h"
#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"
#include "Engine/System.h"

Engine::CameraManager& Engine::CameraManager::GetInstance()
{
    static auto instance = std::make_unique<CameraManager>();
    return *instance;
}

Engine::CameraManager::CameraManager()
{
    
}

glm::vec2 Engine::CameraManager::ConvertWorldToScreen(glm::vec2 worldPos)
{
    const auto screenLocation = glm::vec2(worldPos.x + GetAppWindowSize().x / 2, (worldPos.y + GetAppWindowSize().y / 2));
    return screenLocation;
}

glm::vec2 Engine::CameraManager::ConvertScreenToWorld(glm::vec2 screenPos)
{
    auto worldLocation = glm::vec2(screenPos.x - GetAppWindowSize().x / 2, (screenPos.y - GetAppWindowSize().y / 2) * -1);
    return worldLocation + GetActiveCamera()->gameObject->transform.position;
}
