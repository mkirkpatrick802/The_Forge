#include "CommandTerminal.h"
#include "CommandRegistry.h"

Engine::CommandTerminal::CommandTerminal()
{
    
}

void Engine::CommandTerminal::Render()
{
   // Get the main viewport
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    // Calculate terminal size and position
    const ImVec2 windowSize = ImVec2(400, 150); // Terminal size
    const ImVec2 windowPos = ImVec2(
        mainViewport->Pos.x + 10,                                // Offset 10px from left
        mainViewport->Pos.y + mainViewport->Size.y - windowSize.y - 10 // Offset 10px from bottom
    );

    // Set window properties
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    // Transparent background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));

    if (ImGui::Begin("Command Terminal", nullptr, windowFlags)) {
        // Set up spacing and padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

        // History log
        float inputHeight = ImGui::GetTextLineHeightWithSpacing() + 5; // Input field height
        ImGui::BeginChild("History", ImVec2(0, -inputHeight), false, ImGuiWindowFlags_None); // History log above input
        for (const std::string& line : _outputLines) {
            ImGui::TextUnformatted(line.c_str());
        }
        ImGui::EndChild();

        // Input field at the bottom
        if (ImGui::InputText("##Input", _inputBuffer, sizeof(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string input = _inputBuffer;
            if (!input.empty()) {
                AddOutput("> " + input);                // Add input to history
                CommandRegistry::ExecuteCommand(input);     // Execute command
                _inputBuffer[0] = '\0';                     // Clear the input buffer
            }
            ImGui::SetKeyboardFocusHere(-1);          // Keep focus on the input field
        }

        ImGui::PopStyleVar(2); // Restore spacing and padding
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

void Engine::CommandTerminal::AddOutput(const String& line)
{
    _outputLines.push_back(line);
}

void Engine::CommandTerminal::ClearOutput()
{
    _outputLines.clear();
}
