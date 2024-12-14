#pragma once
#include <vector>

#include "Data.h"
#include "UIWindow.h"

namespace Engine
{
    class CommandTerminal final: public UIWindow
    {
    public:

        CommandTerminal();
        void Render() override;

    private:

        void AddOutput(const String& line);
        void ClearOutput();
        
    private:
        char _inputBuffer[256]{};
        std::vector<String> _outputLines;
    };
}
