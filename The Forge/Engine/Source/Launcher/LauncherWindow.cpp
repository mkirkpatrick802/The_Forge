#include "LauncherWindow.h"

#include "imgui_internal.h"

void LauncherWindow::Render()
{
    static auto once = true;
    if (once)
    {
        once = false;

        // Docking setup
        const ImGuiID ID = ImGui::GetID("Launcher");

        static ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockBuilderRemoveNode(ID); // clear any previous layout
        ImGui::DockBuilderAddNode(ID, flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(ID, ImGui::GetIO().DisplaySize);

        ImGui::DockBuilderDockWindow("Launcher", ID);
        ImGui::DockBuilderFinish(ID);
    }

    // Create a window
    ImGui::Begin("Launcher", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Text("Yo");

    ImGui::End();
}
