#include "ChatWindow.h"

#include "CommandRegistry.h"
#include "EventData.h"
#include "EventSystem.h"
#include "System.h"

Engine::ChatWindow::ChatWindow()
{
    EventSystem::GetInstance()->RegisterEvent(ED_LogToConsole::GetEventName(), this, &ChatWindow::AddToLog);
}

Engine::ChatWindow::~ChatWindow()
{
    EventSystem::GetInstance()->DeregisterEvent(ED_LogToConsole::GetEventName(), this);
}

void Engine::ChatWindow::Render()
{
    // Get the main viewport
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    // Calculate history size and position
    _lineHeight = ImGui::GetTextLineHeight() + 5; // Input field height
    float x = 300;
    float y = max(_lineHeight, (float)_outputBuffer.size() * _lineHeight) + 15;
    y = min(10 * _lineHeight - 5, y);
    ImVec2 windowSize = ImVec2(x, y);
    
    const ImVec2 windowPos = ImVec2(
        mainViewport->Pos.x + 10,                                  
        mainViewport->Pos.y + mainViewport->Size.y - windowSize.y - _lineHeight - 15
    );

    // Transparent background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

    // Set up spacing and padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    
    // Set window properties
    if (!_outputBuffer.empty())
    {
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        History();
    }

    ImGui::PopStyleColor();

    if (_terminalActive)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, .5f));
        ImGui::SetNextWindowSize(ImVec2(x, _lineHeight));
        ImGui::SetNextWindowPos(ImVec2(mainViewport->Pos.x + 10, mainViewport->Pos.y + mainViewport->Size.y - _lineHeight - 15));
        Terminal();
        
        ImGui::PopStyleColor();
    }
    
    ImGui::PopStyleVar(3);
}

void Engine::ChatWindow::History() const
{
    if (ImGui::Begin("History", nullptr, _windowFlags))
    {
        for (const auto& [message, type] : _outputBuffer)
        {
            ImVec4 color; // Variable to hold color based on log type

            switch (type)
            {
            case LogType::MESSAGE_LOG:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for normal messages
                break;
                
            case LogType::WARNING_LOG:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for warnings
                break;
                
            case LogType::ERROR_LOG:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for errors
                break;
            }

            // Apply color and display the message
            ImGui::TextColored(color, "> %s", message.c_str());
        }

        // Automatically scroll to the bottom when new entries are added
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1);
    }
    
    ImGui::End();
}

void Engine::ChatWindow::Terminal()
{
    if (ImGui::Begin("Terminal", nullptr, _windowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();

        float x = (windowSize.x - ImGui::GetItemRectSize().x) * .5f;
        float y = (windowSize.y - ImGui::GetItemRectSize().y) * .5f;

        ImGui::SetCursorPos(ImVec2(x + 10, y));
        
        // Input field at the bottom
        ImGui::SetKeyboardFocusHere(1);
        ImGui::SetNextItemWidth(290);
        if (ImGui::InputText("##Input", _inputBuffer, sizeof(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string input = _inputBuffer;
            if (!input.empty()) {
                AddOutput(input);           // Add input to history
                CommandRegistry::ExecuteCommand(input); // Execute command
                _inputBuffer[0] = '\0';            // Clear the input buffer
            }
            ImGui::SetKeyboardFocusHere(-1);      // Keep focus on the input field
        }
    }
    
    ImGui::End();
}

void Engine::ChatWindow::AddOutput(const std::string& line)
{
    _outputBuffer.push_back(std::make_pair(line, LogType::MESSAGE_LOG));
}

void Engine::ChatWindow::AddToLog(const void* p)
{
    const auto eventData = static_cast<const ED_LogToConsole*>(p); 
    if (!eventData) return;
    
    _outputBuffer.emplace_back(eventData->message, eventData->type);
}

void Engine::ChatWindow::ClearOutput()
{
    _outputBuffer.clear();
}
