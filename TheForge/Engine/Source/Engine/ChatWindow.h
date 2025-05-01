#pragma once
#include <string>
#include <vector>

#include "Rendering/UIWindow.h"

namespace Engine
{
    enum class LogType : uint8_t;

    class ChatWindow final : public UIWindow
    {
    public:
        ChatWindow();
        ~ChatWindow();
        
        void Render() override;

        void EnableTerminal(const bool enable) {_terminalActive = enable;}
        
    private:
        void History() const;
        void Terminal();

        void AddOutput(const std::string& line);
        void AddToLog(const void* p);
        void ClearOutput();
        
    private:
        char _inputBuffer[256]{};
        std::vector<std::pair<std::string, LogType>> _outputBuffer;

        ImGuiWindowFlags _windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        float _lineHeight;
        bool _terminalActive = false;
    };
}
