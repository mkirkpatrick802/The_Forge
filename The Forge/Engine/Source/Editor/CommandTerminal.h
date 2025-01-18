#pragma once
#include <vector>

#include "Engine/Data.h"
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
        
        void AddOutput(const String& line);
        void ClearOutput();
        
    private:
        char _inputBuffer[256]{};
        static std::vector<String> _outputLines;
        bool _scrollToBottom = true;
        
    };
}
