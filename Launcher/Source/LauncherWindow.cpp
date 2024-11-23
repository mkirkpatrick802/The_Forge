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

        // Docking Setup
        const ImGuiID ID = ImGui::GetID("Launcher");

        static ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockBuilderRemoveNode(ID); // clear any previous layout
        ImGui::DockBuilderAddNode(ID, flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(ID, ImGui::GetIO().DisplaySize);

        ImGui::DockBuilderDockWindow("Launcher", ID);
        ImGui::DockBuilderFinish(ID);

        // Load Textures
        _iconTexture = Engine::LoadTexture("Assets/Sprites/icon.png", _iconSize);
    }

    DrawMenu();
}

void LauncherWindow::DrawMenu()
{
    if (_settings == nullptr)
        Engine::System::DisplayMessageBox("Error!", "Launchers Settings not set on launcher window");

    // Create a window
    ImGui::Begin("Launcher", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    const ImGuiStyle& style = ImGui::GetStyle();
    
    ImGui::End();
}
