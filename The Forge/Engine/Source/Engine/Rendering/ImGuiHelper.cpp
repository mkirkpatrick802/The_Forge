#include "ImGuiHelper.h"

#include "imgui.h"

bool Engine::ImGuiHelper::CenteredButtonWithPadding(const std::string& label, float verticalPadding)
{
    const ImVec2 windowSize = ImGui::GetWindowSize();  // Get the size of the current window
    ImVec2 windowPos = ImGui::GetWindowPos();    // Get the position of the current window
    const float windowWidth = windowSize.x;

    // Calculate the size of the button
    ImVec2 buttonSize = ImGui::CalcTextSize(label.c_str());
    buttonSize.x += ImGui::GetStyle().FramePadding.x * 2; // Include padding in width
    buttonSize.y += ImGui::GetStyle().FramePadding.y * 2; // Include padding in height

    // Center horizontally
    const float buttonX = (windowWidth - buttonSize.x) / 2.0f;

    // Add vertical padding before rendering
    ImGui::Dummy(ImVec2(0.0f, verticalPadding));  // Adds vertical space

    // Set cursor position to center button
    ImGui::SetCursorPosX(buttonX);

    // Render the button
    if (ImGui::Button(label.c_str()))
        return true;

    return false;
}

bool Engine::ImGuiHelper::DragDropFileButton(const std::string& label, std::string& filePath, const char* payloadType)
{
    bool updated = false;
    if (ImGui::Button(label.c_str(), ImVec2(50, 50))) {}

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType))
        {
            filePath = static_cast<const char*>(payload->Data);
            updated = true;
        }
        ImGui::EndDragDropTarget();
    }
    return updated;
}

void Engine::ImGuiHelper::DisplayFilePath(const std::string& label, const std::string& filePath)
{
    if (!filePath.empty())
    {
        ImGui::Text("%s: %s", label.c_str(), filePath.c_str());
    }
}

void Engine::ImGuiHelper::InputVector2(const std::string& label, glm::vec2& input)
{
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();
    ImGui::PushItemWidth(50);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    std::string xlabel = "##X" + label;
    std::string ylabel = "##Y" + label;
    ImGui::InputFloat(xlabel.c_str(), &input.x, 0, 0);
    ImGui::SameLine();
    ImGui::InputFloat(ylabel.c_str(), &input.y, 0, 0);
    ImGui::PopItemWidth();
}
