#pragma once

namespace JsonKeywords
{
    namespace Config
    {
        const std::string DEFAULT_LEVEL = "Default Level";
    }
    
    const std::string LEVEL_NAME = "Level Name";

    const std::string GAMEOBJECT_ARRAY = "Level Game Objects";
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