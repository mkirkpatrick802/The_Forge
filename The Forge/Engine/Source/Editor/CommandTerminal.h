#pragma once
#include <string>
#include <vector>

#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    class CommandTerminal final: public Engine::UIWindow
    {
    public:

        CommandTerminal();
        ~CommandTerminal();
        
        void Render() override;

    private:
        
        void AddOutput(const std::string& line);
        void ClearOutput();
        
    private:
        char _inputBuffer[256]{};
        static std::vector<std::string> _outputLines;
        bool _scrollToBottom = true;
        
    };
}
