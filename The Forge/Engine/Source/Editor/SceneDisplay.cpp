#include "SceneDisplay.h"

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

        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::Image(
            (ImTextureID)sceneFBO->GetTextureID(),
            ImVec2(sceneFBO->GetSize().x, sceneFBO->GetSize().y),
            ImVec2(0, 1), ImVec2(1, 0)
            );

        // Check for resize based on window size
        if (const Vector2D windowSize(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y); sceneFBO->GetSize() != windowSize)
            sceneFBO->Resize(windowSize);

        ImGui::EndChild();
    }
    
    ImGui::End();
}
