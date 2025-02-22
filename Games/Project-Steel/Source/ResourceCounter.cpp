#include "ResourceCounter.h"

#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/Renderer.h"

void ResourceCounter::UpdateResourceCounter(const int resourceCount)
{
    const std::string text = "Resources: " + std::to_string(resourceCount);
    SetText(text);
}

void ResourceCounter::Deserialize(const json& data)
{
    if (data.contains(JsonKeywords::TEXT_RENDERER_TEXT))
        _text = data[JsonKeywords::TEXT_RENDERER_TEXT];

    if (data.contains(JsonKeywords::TEXT_RENDERER_SIZE))
        _fontSize = data[JsonKeywords::TEXT_RENDERER_SIZE];

    if (data.contains(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER))
        sortingLayer = data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER];

    if (data.contains(JsonKeywords::TEXT_RENDERER_SCREEN_POS_X) && data.contains(JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y))
    {
        _screenPos.x = data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_X];
        _screenPos.y = data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y];
    }

    _font = std::make_unique<Engine::Font>("Assets/Engine Assets/Fonts/Consolas.ttf", _fontSize);
    Engine::GetRenderer().AddComponentToRenderList(this);
}

nlohmann::json ResourceCounter::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = Engine::GetComponentRegistry().GetComponentID<ResourceCounter>();
    data[JsonKeywords::TEXT_RENDERER_TEXT] = _text;
    data[JsonKeywords::TEXT_RENDERER_SIZE]= _fontSize;
    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = sortingLayer;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_X] = _screenPos.x;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y] = _screenPos.y;
    return data;
}
