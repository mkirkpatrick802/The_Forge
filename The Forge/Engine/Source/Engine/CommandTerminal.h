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

        static void CleanUp();
        void Render() override;

    private:

        void History();
        void Terminal();
        
        void AddOutput(const String& line);
        void ClearOutput();
        
    private:
        char _inputBuffer[256]{};
        static std::vector<String> _outputLines;

        ImGuiWindowFlags _windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        float _lineHeight;
    };
}
