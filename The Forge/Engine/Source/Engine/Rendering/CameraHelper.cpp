#include "CameraHelper.h"

#include "CameraManager.h"
#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"

glm::mat4 Engine::GetProjectionMatrix()
{
    if (EngineManager::IsEditorEnabled())
        return Editor::EditorCamera::GetInstance()->GetProjectionMatrix();

    if (const auto activeCamera = CameraManager::GetCameraManager()->GetActiveCamera())
        return activeCamera->GetProjectionMatrix();

    return {};
}

glm::mat4 Engine::GetViewMatrix()
{
    if (EngineManager::IsEditorEnabled())
    {
        return Editor::EditorCamera::GetInstance()->GetViewMatrix();
    }
    
    if (const auto activeCamera = CameraManager::GetCameraManager()->GetActiveCamera())
        return activeCamera->GetViewMatrix();

    return {};
}
