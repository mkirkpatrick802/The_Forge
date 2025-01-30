#pragma once

#include <array>
#include <memory>

#include "Engine/Data.h"
#include "Engine/Rendering/UIWindow.h"

namespace Editor
{

    const uint8 MAX_EDITOR_WINDOW_COUNT = 10;
    const uint8 LEVEL_EDITOR_INDEX = 1;
    const uint8 DETAILS_EDITOR_INDEX = 2;
    const uint8 SCENE_DISPLAY = 3;
    const uint8 COMMAND_TERMINAL = 4;
    const uint8 CONSOLE = 5;
    const uint8 CONTENT_DRAWER = 6;
    
    // This class will hold a list of all the editor panels and keep track of which ones are active or not.
    // This should be the only editor class to talk with the UI manager class
    class EditorManager
    {
    public:

        EditorManager();
        ~EditorManager();
    
    private:

        // First use of smart pointers!!!
        std::array<std::shared_ptr<Engine::UIWindow>, MAX_EDITOR_WINDOW_COUNT> EditorWindows;
    };
}
