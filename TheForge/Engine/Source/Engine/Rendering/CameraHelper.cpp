#include "CameraHelper.h"

#include "CameraManager.h"
#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"

glm::mat4 Engine::GetProjectionMatrix()
{
    if (GetEngineManager().IsEditorEnabled())
        return Editor::EditorCamera::GetInstance()->GetProjectionMatrix();

    if (const auto activeCamera = GetCameraManager().GetActiveCamera())
        return activeCamera->GetProjectionMatrix();

    // Identity, not {}. A default-constructed glm::mat4 is all zeroes, which multiplies
    // every vertex to the origin and renders a perfectly black screen -- the failure
    // looks like "the renderer is broken" rather than "there is no camera".
    return glm::mat4(1.0f);
}

glm::mat4 Engine::GetViewMatrix()
{
    if (GetEngineManager().IsEditorEnabled())
    {
        return Editor::EditorCamera::GetInstance()->GetViewMatrix();
    }
    
    if (const auto activeCamera = GetCameraManager().GetActiveCamera())
        return activeCamera->GetViewMatrix();

    return glm::mat4(1.0f);
}
