#include "LauncherWindow.h"

#include <imgui_internal.h>

#include "Launcher.h"
#include "Engine/TextureLoader.h"

LauncherWindow::LauncherWindow() = default;

void LauncherWindow::Render()
{
    static auto once = true;
    if (once)
    {
        once = false;
        
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
        static ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;
    }
        
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

    ImGui::Text("This window is locked to the app window!");
    
    ImGui::End();
}
