#include "Console.h"

#include "Engine/EventData.h"
#include "Engine/EventSystem.h"
#include "Engine/System.h"

Editor::Console::Console()
{
    Engine::EventSystem::GetInstance()->RegisterEvent(Engine::ED_LogToConsole::GetEventName(), this, &Console::AddToLog);
    DEBUG_PRINT("Hello Debug Console")
}

Editor::Console::~Console()
{
    Engine::EventSystem::GetInstance()->DeregisterEvent(Engine::ED_LogToConsole::GetEventName());

    _log.clear();
    _log.shrink_to_fit();
}

void Editor::Console::Render()
{
    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Add a scrollable region
    if (ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        if (!_log.empty())
        {
            for (auto [message, type] : _log)
            {
                ImVec4 color; // Variable to hold color based on log type

                switch (type)
                {
                default:
                case Engine::LogType::MESSAGE_LOG:
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for normal messages
                    break;
                case Engine::LogType::WARNING_LOG:
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for warnings
                    break;
                case Engine::LogType::ERROR_LOG:
                    color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for errors
                    break;
                }

                // Apply color and display the message
                ImGui::TextColored(color, "> %s", message.c_str());
            }

            // Auto-scroll to the bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    // Add a clear button
    if (ImGui::Button("Clear"))
    {
        _log.clear();
    }

    // Add a dummy input for user interaction
    ImGui::SameLine();
    if (ImGui::Button("Print to File"))
    {
        // Placeholder: Add copy-to-clipboard functionality
        // e.g., ImGui::LogToClipboard();
    }

    ImGui::End();
}

void Editor::Console::AddToLog(const void* data)
{
    const auto eventData = static_cast<const Engine::ED_LogToConsole*>(data); 
    if (!eventData) return;
    
    _log.emplace_back(eventData->message, eventData->type);
}
