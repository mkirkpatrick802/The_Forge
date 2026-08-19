#include "ResourceCounter.h"

#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Components/PlayerController.h"
#include "Engine/Rendering/Renderer.h"

void ResourceCounter::Render(const Engine::ShaderUniformData& data)
{
    if (const auto controller = gameObject->GetComponent<Engine::PlayerController>())
    {
        if (!controller->IsLocalPlayer()) return;
        TextRenderer::Render(data);
    }
}

void ResourceCounter::UpdateResourceCounter(const int resourceCount)
{
    const std::string text = "Resources: " + std::to_string(resourceCount);
    SetText(text);
}

nlohmann::json ResourceCounter::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::TEXT_RENDERER_TEXT] = _text;
    data[JsonKeywords::TEXT_RENDERER_SIZE]= _fontSize;
    //data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = sortingLayer;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_X] = _screenPos.x;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y] = _screenPos.y;
    return data;
}
