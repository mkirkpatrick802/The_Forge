#include "CommandTerminal.h"

#include "Engine/CommandRegistry.h"

std::vector<String> Editor::CommandTerminal::_outputLines;

Editor::CommandTerminal::CommandTerminal()
{
    
}

Editor::CommandTerminal::~CommandTerminal()
{
    _outputLines.clear();
    _outputLines.shrink_to_fit();
}

void Editor::CommandTerminal::Render()
{
    if (ImGui::Begin("Command Terminal"))
    {
        // Output Area
        if (ImGui::BeginChild("Output", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 2), true))
        {
            for (const auto& line : _outputLines) 
                ImGui::TextUnformatted(line.c_str());
            
            if (_scrollToBottom)
                ImGui::SetScrollHereY(1.0f);
            
            _scrollToBottom = false;
        }
        
        ImGui::EndChild();

        // Input Area
        ImGui::Separator();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##Input", _inputBuffer, IM_ARRAYSIZE(_inputBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue,
                             nullptr, nullptr))
        {
            Engine::CommandRegistry::ExecuteCommand(_inputBuffer);
            const std::string input = _inputBuffer;
            AddOutput("> " + input);
            _inputBuffer[0] = '\0'; // Clear input buffer
        }

        // Handle focus
        if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
            ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

/*void Editor::CommandTerminal::Terminal()
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
                Engine::CommandRegistry::ExecuteCommand(input); // Execute command
                _inputBuffer[0] = '\0';            // Clear the input buffer
            }
            ImGui::SetKeyboardFocusHere(-1);      // Keep focus on the input field
        }
    }
    
    ImGui::End();
}*/

void Editor::CommandTerminal::AddOutput(const String& line)
{
    _outputLines.push_back(line);
}

void Editor::CommandTerminal::ClearOutput()
{
    _outputLines.clear();
}
