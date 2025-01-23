#include "CameraHelper.h"

#include "CameraManager.h"
#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"

glm::mat4 Engine::GetProjectionMatrix()
{
    if (EngineManager::IsEditorEnabled())
    {
        return Editor::EditorCamera::GetInstance()->GetProjectionMatrix();
    }
    else
    {
        return CameraManager::GetCameraManager()->GetActiveCamera()->GetProjectionMatrix();
    }
}

glm::mat4 Engine::GetViewMatrix()
{
    if (EngineManager::IsEditorEnabled())
    {
        return Editor::EditorCamera::GetInstance()->GetViewMatrix();
    }
    
    return {};
}
