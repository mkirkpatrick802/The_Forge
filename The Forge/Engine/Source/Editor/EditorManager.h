﻿#pragma once

#include <array>
#include <memory>

#include "Engine/Rendering/UIWindow.h"

namespace Editor
{

    const uint8_t MAX_EDITOR_WINDOW_COUNT = 10;
    const uint8_t LEVEL_EDITOR_INDEX = 1;
    const uint8_t DETAILS_EDITOR_INDEX = 2;
    const uint8_t SCENE_DISPLAY = 3;
    const uint8_t COMMAND_TERMINAL = 4;
    const uint8_t CONSOLE = 5;
    const uint8_t CONTENT_DRAWER = 6;
    const uint8_t MENU_BAR = 7;
    
    // This class will hold a list of all the editor panels and keep track of which ones are active or not.
    // This should be the only editor class to talk with the UI manager class
    class EditorManager
    {
    public:

        EditorManager();
        ~EditorManager();

        void Update();
    
    private:

        // First use of smart pointers!!!
        std::array<std::shared_ptr<Engine::UIWindow>, MAX_EDITOR_WINDOW_COUNT> EditorWindows;
    };
}
