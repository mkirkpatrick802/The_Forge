#include "ImGuiHelper.h"

#include "imgui.h"

bool Editor::ImGuiHelper::CenteredButtonWithPadding(const char* label, float verticalPadding)
{
    const ImVec2 windowSize = ImGui::GetWindowSize();  // Get the size of the current window
    ImVec2 windowPos = ImGui::GetWindowPos();    // Get the position of the current window
    const float windowWidth = windowSize.x;

    // Calculate the size of the button
    ImVec2 buttonSize = ImGui::CalcTextSize(label);
    buttonSize.x += ImGui::GetStyle().FramePadding.x * 2; // Include padding in width
    buttonSize.y += ImGui::GetStyle().FramePadding.y * 2; // Include padding in height

    // Center horizontally
    const float buttonX = (windowWidth - buttonSize.x) / 2.0f;

    // Add vertical padding before rendering
    ImGui::Dummy(ImVec2(0.0f, verticalPadding));  // Adds vertical space

    // Set cursor position to center button
    ImGui::SetCursorPosX(buttonX);

    // Render the button
    if (ImGui::Button(label))
        return true;

    return false;
}
