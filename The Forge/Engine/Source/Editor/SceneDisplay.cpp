﻿#include "SceneDisplay.h"

#include "EditorCamera.h"
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
        ImGui::Image(
            (ImTextureID)sceneFBO->GetTextureID(),
            ImVec2(sceneFBO->GetSize().x, sceneFBO->GetSize().y),
            ImVec2(0, 1), ImVec2(1, 0)
            );
        
        // Check for resize based on window size
        const glm::vec2 windowSize(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y); 
        if (sceneFBO->GetSize() != windowSize)
            sceneFBO->Resize(windowSize);
    }

    ImGui::EndChild();
    ImGui::End();
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
