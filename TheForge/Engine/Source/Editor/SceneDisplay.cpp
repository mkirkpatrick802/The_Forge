#include "SceneDisplay.h"

#include "EditorCamera.h"
#include "LevelEditor.h"
#include "ScenePicker.h"
#include "Engine/System.h"
#include "Engine/Rendering/BufferRegistry.h"


Editor::SceneDisplay::SceneDisplay()
{
    
}

void Editor::SceneDisplay::Render()
{
    ImGui::Begin("Scene");

    if (ImGui::BeginChild("##SceneChild", ImVec2(0, 0), 0, ImGuiWindowFlags_NoScrollWithMouse))
    {
        auto sceneFBO = Engine::BufferRegistry::GetRegistry()->GetBuffer(Engine::BufferRegistry::BufferType::SCENE);

        ClickAndDrag();
        
        ImGui::SetCursorPos(ImVec2(0, 0));

        // Captured before the image is submitted, because GetCursorScreenPos is where
        // the *next* item goes. This is the only reliable origin for the scene image:
        // it is inside a child window at an arbitrary dock position.
        const ImVec2 imageOrigin = ImGui::GetCursorScreenPos();

        ImGui::Image(
            (ImTextureID)sceneFBO->GetTextureID(),
            ImVec2(sceneFBO->GetSize().x, sceneFBO->GetSize().y),
            ImVec2(0, 1), ImVec2(1, 0)
            );

        HandleSelection(glm::vec2(imageOrigin.x, imageOrigin.y), sceneFBO->GetSize());

        // Check for resize based on window size
        const glm::vec2 windowSize(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y); 
        if (sceneFBO->GetSize() != windowSize)
            sceneFBO->Resize(windowSize);
    }

    ImGui::EndChild();
    ImGui::End();
}

void Editor::SceneDisplay::HandleSelection(const glm::vec2 imageOrigin, const glm::vec2 imageSize)
{
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) return;

    const glm::vec2 mouse(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        _pressPosition = mouse;

    if (!ImGui::IsMouseReleased(ImGuiMouseButton_Left)) return;

    // Left-drag already pans the camera in this same window, so only a press that
    // barely moved counts as a selection.
    if (glm::distance(mouse, _pressPosition) > CLICK_TOLERANCE_PIXELS) return;

    const glm::vec2 viewportPixels = mouse - imageOrigin;
    if (viewportPixels.x < 0.0f || viewportPixels.y < 0.0f ||
        viewportPixels.x >= imageSize.x || viewportPixels.y >= imageSize.y)
        return;

    // Clicking the same place again steps to whatever is behind the current pick.
    const bool sameSpot = glm::distance(mouse, _lastClickPosition) <= CLICK_TOLERANCE_PIXELS;
    _lastClickPosition = mouse;

    const glm::vec2 world = EditorCamera::GetInstance()->ViewportToWorld(viewportPixels);

    // Nothing under the cursor clears the selection, which is how the Details panel is
    // dismissed.
    LevelEditor::SelectGameObject(ScenePicker::Pick(world, sameSpot));
}

void Editor::SceneDisplay::ClickAndDrag()
{
    if (ImGui::IsWindowHovered())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const auto dragDelta = glm::vec2(ImGui::GetMouseDragDelta().x, ImGui::GetMouseDragDelta().y);
            
            EditorCamera::GetInstance()->UpdatePosition(dragDelta);
            
            ImGui::ResetMouseDragDelta();
        }
    }
}
