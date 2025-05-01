#include "MenuBar.h"

#include "Engine/GameEngine.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"

void Editor::MenuBar::Render()
{
    if (ImGui::BeginMainMenuBar())
    {
        float menuHeight = ImGui::GetWindowHeight(); // Get menu bar height
        float menuWidth = ImGui::GetWindowWidth();
        float buttonWidth = 80.0f; // Button width
        float buttonHeight = ImGui::GetFrameHeight(); // Ensure it matches ImGui's default height

        // Left-aligned menu items
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save")) { Engine::LevelManager::GetCurrentLevel()->SaveLevel(); }
            ImGui::EndMenu();
        }

        // Centering logic (X and Y adjustments)
        float buttonX = (menuWidth - buttonWidth) * 0.5f;
        float buttonY = (menuHeight - buttonHeight) * 0.5f; // Center vertically

        ImGui::SetCursorPosX(buttonX);
        ImGui::SetCursorPosY(buttonY);
        if (ImGui::Button("Play", ImVec2(buttonWidth, buttonHeight))) 
        {
            Engine::GetLevelManager().StartCurrentLevel();
        }

        // Right-aligned menu items
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Options")) { /* Handle Options */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
