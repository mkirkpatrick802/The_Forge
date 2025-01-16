#include "LauncherWindow.h"
#include <imgui_internal.h>
#include "Launcher.h"
#include "Engine/TextureLoader.h"

LauncherWindow::LauncherWindow() = default;

void LauncherWindow::Render()
{
    DrawMenu();
}

void LauncherWindow::DrawMenu()
{
    if (_settings == nullptr)
        Engine::System::DisplayMessageBox("Error!", "Launchers Settings not set on launcher window");

    ImGuiIO& io = ImGui::GetIO();
    
    // Create a window
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Launcher", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    const ImGuiStyle& style = ImGui::GetStyle();

    // Convert std::vector<std::string> to const char* array
    std::vector<const char*> cstrProjects;
    for (const auto& project : _settings->projects) {
        cstrProjects.push_back(project.c_str());
    }
    
    int item = 0;
    if (ImGui::Combo("Selected Project", &item, cstrProjects.data(), cstrProjects.size()))
    {
        
    }
    
    ImGui::End();
}
