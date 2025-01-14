#include "CommandTerminal.h"
#include "CommandRegistry.h"

std::vector<String> Engine::CommandTerminal::_outputLines;

Engine::CommandTerminal::CommandTerminal()
{
    
}

void Engine::CommandTerminal::CleanUp()
{
    _outputLines.clear();
    _outputLines.shrink_to_fit();
}

void Engine::CommandTerminal::Render()
{
    // Get the main viewport
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    // Calculate history size and position
    _lineHeight = ImGui::GetTextLineHeight() + 5; // Input field height
    float x = 300;
    float y = _outputLines.size() > 5 ? 5 * _lineHeight : _outputLines.size() * _lineHeight;
    const ImVec2 windowSize = ImVec2(x, y); // Terminal size
    const ImVec2 windowPos = ImVec2(
        mainViewport->Pos.x + 10,                                       // Offset 10px from left
        mainViewport->Pos.y + mainViewport->Size.y - windowSize.y - (15 + _lineHeight + (_outputLines.size() == 1 ? 14 : 0))  // Offset 10px from bottom
    );

    // Transparent background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));

    // Set up spacing and padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    
    // Set window properties
    if (!_outputLines.empty())
    {
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        History();
    }

    ImGui::SetNextWindowSize(ImVec2(x, _lineHeight));
    ImGui::SetNextWindowPos(ImVec2(mainViewport->Pos.x + 10, mainViewport->Pos.y + mainViewport->Size.y - _lineHeight - 15));
    Terminal();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void Engine::CommandTerminal::History()
{
    if (ImGui::Begin("History", nullptr, _windowFlags))
    {
        // Render all history lines in natural order
        ImGui::Spacing();
        
        for (const std::string& line : _outputLines)
            ImGui::TextUnformatted(line.c_str());

        // Automatically scroll to the bottom when new entries are added
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1);
    }
    
    ImGui::End();
}

void Engine::CommandTerminal::Terminal()
{
    if (ImGui::Begin("Terminal", nullptr, _windowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();

        float x = (windowSize.x - ImGui::GetItemRectSize().x) * .5f;
        float y = (windowSize.y - ImGui::GetItemRectSize().y) * .5f;

        ImGui::SetCursorPos(ImVec2(x + 10, y));
        // Input field at the bottom
        ImGui::SetKeyboardFocusHere(1);
        if (ImGui::InputText("##Input", _inputBuffer, sizeof(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string input = _inputBuffer;
            if (!input.empty()) {
                AddOutput("> " + input);           // Add input to history
                CommandRegistry::ExecuteCommand(input); // Execute command
                _inputBuffer[0] = '\0';            // Clear the input buffer
            }
            ImGui::SetKeyboardFocusHere(-1);      // Keep focus on the input field
        }
    }
    
    ImGui::End();
}

void Engine::CommandTerminal::AddOutput(const String& line)
{
    _outputLines.push_back(line);
}

void Engine::CommandTerminal::ClearOutput()
{
    _outputLines.clear();
}
