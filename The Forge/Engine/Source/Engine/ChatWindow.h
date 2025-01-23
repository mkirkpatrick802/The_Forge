#pragma once
#include <vector>

#include "Data.h"
#include "Rendering/UIWindow.h"

namespace Engine
{
    class ChatWindow final : public UIWindow
    {
    public:
        
        ChatWindow() = default;
        void CleanUp();
        
        void Render() override;

    private:

        void History();
        void Terminal();

        void AddOutput(const String& line);
        void ClearOutput();
        
    private:

        // TODO: This should be tracked in the command registry or smth else,
        //       so commands can be saved across editor and game
        char _inputBuffer[256]{};
        static std::vector<String> _outputLines;

        ImGuiWindowFlags _windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        float _lineHeight;
    };
}
