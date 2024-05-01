#include "LauncherWindow.h"

#include <imgui_internal.h>
#include "Engine/TextureLoader.h"

LauncherWindow::LauncherWindow()
{
	iconTexture = Engine::LoadTexture("Assets/Sprites/icon.png", iconSize);
}

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

    DrawMenu();
}

void LauncherWindow::DrawMenu()
{
    // Create a window
    ImGui::Begin("Launcher", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    const ImGuiStyle& style = ImGui::GetStyle();

    {
        const float size = iconSize.x + style.FramePadding.x * 2.0f;
        const float avail = ImGui::GetContentRegionAvail().x;

        const float off = (avail - size) * .5;
        if (off > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        // Icon above elements
        if (iconTexture != 0)
            ImGui::Image((void*)(intptr_t)iconTexture, ImVec2(iconSize.x, iconSize.y));
    }

    ImGui::Separator();

    ImGui::BeginColumns("Options", 2, ImGuiOldColumnFlags_NoResize);
    ImGui::SetColumnWidth(0, ImGui::GetIO().DisplaySize.x / 2);

    // Center the "Play Mode" button in the first column
    float buttonWidth1 = ImGui::CalcTextSize("Play Mode").x;
    float offset1 = (ImGui::GetColumnWidth() - buttonWidth1) / 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset1);
    if (ImGui::Button("Play Mode")) { currentMode = Mode::Play; }

    ImGui::NextColumn();

    // Center the "Edit Mode" button in the second column
    float buttonWidth2 = ImGui::CalcTextSize("Edit Mode").x;
    float offset2 = (ImGui::GetColumnWidth() - buttonWidth2) / 2.0f - 20;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset2);
    if (ImGui::Button("Edit Mode")) { currentMode = Mode::Edit; }

    ImGui::NextColumn();

    // Play mode options
    if (currentMode == Mode::Play)
    {
        ImGui::Dummy(ImVec2(1, 10));
        ImGui::Checkbox("Host", &isHosting);
        if (!isHosting)
        {
            char buffer[256];
            strcpy_s(buffer, address.c_str());
            ImGui::Dummy(ImVec2(1, 5));
            ImGui::Text("Enter IPv4:");
            ImGui::InputText(" ", buffer, sizeof(buffer));
            address = buffer;
        }

        float buttonWidth1 = 90;
        float offset1 = (ImGui::GetColumnWidth() - buttonWidth1) / 2.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset1);
        ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetStyle().FramePadding.y - ImGui::GetFrameHeightWithSpacing() * 2);

        if (ImGui::Button("Start", ImVec2(90, 20))) 
        {
            // Handle button click
        }
    }
    else
    {
        ImGui::NextColumn();

        float buttonWidth2 = 90;
        float offset2 = (ImGui::GetColumnWidth() - buttonWidth2) / 2.0f - 15;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset2);
        ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetStyle().FramePadding.y - ImGui::GetFrameHeightWithSpacing() * 2);

        if (ImGui::Button("Start", ImVec2(90, 20)))
        {
            // Handle button click
        }
    }

    ImGui::EndColumns();
    ImGui::End();
}
