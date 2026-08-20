#pragma once

namespace JsonKeywords
{
    namespace Config
    {
        const std::string DEFAULT_LEVEL = "Default Level";

        // The level a windowed client boots into. Absent means "no menu" -- boot
        // straight into DEFAULT_LEVEL, which is what every build did before menus
        // existed and is still what a dedicated server and the editor do.
        const std::string MAIN_MENU_LEVEL = "Main Menu Level";

        // Where Play connects to. A text field on the menu would override it; until
        // there is one, this is how you point a build at a server.
        const std::string SERVER_ADDRESS = "Server Address";
        const std::string SERVER_PORT = "Server Port";
    }
    
    const std::string LEVEL_NAME = "Level Name";

    const std::string GAMEOBJECT_ARRAY = "Level Game Objects";
    const std::string LEVEL_GAME_MODE = "Game Mode";
    const std::string GAMEOBJECT_NAME = "Game Object Name";
    const std::string GAMEOBJECT_ISREPLICATED = "Is Replicated";
    const std::string GAMEOBJECT_POSITION_X = "X position";
    const std::string GAMEOBJECT_POSITION_Y = "Y position";
    const std::string GAMEOBJECT_ROTATION = "Rotation";
    
    const std::string COMPONENT_ARRAY = "Components";
    const std::string COMPONENT_ID = "Component ID";

    const std::string SPRITE_RENDERER_SPRITE = "Sprite";
    const std::string SPRITE_RENDERER_SORTING_LAYER = "Sorting Layer";
    const std::string SPRITE_RENDERER_VERTEX_SHADER = "Vertex Shader";
    const std::string SPRITE_RENDERER_FRAGMENT_SHADER = "Fragment Shader";

    const std::string TEXT_RENDERER_TEXT = "Text";
    const std::string TEXT_RENDERER_SIZE = "Size";
    const std::string TEXT_RENDERER_SCREEN_POS_X = "Screen Position X";
    const std::string TEXT_RENDERER_SCREEN_POS_Y = "Screen Position Y";
}