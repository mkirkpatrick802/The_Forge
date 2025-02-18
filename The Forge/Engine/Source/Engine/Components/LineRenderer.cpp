#include "LineRenderer.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::LineRenderer::LineRenderer(): _start(), _end()
{
    
}

void Engine::LineRenderer::Render()
{
    
}

void Engine::LineRenderer::Deserialize(const json& data)
{
    if (data.contains(JsonKeywords::SPRITE_RENDERER_SPRITE))
    {
        const std::string filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
        _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
        _size = _texture->GetSize();
    }

    std::string vertex = "Assets/Engine Assets/Shaders/Sprite.vert";
    std::string fragment = "Assets/Engine Assets/Shaders/Sprite.frag";

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = (int16_t)data.value(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER, 0);
}

nlohmann::json Engine::LineRenderer::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<LineRenderer>();

    if (_texture)
    {
        std::string filepath = _texture->GetFilePath();
        data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    }

    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = _sortingLayer;
    
    return data;
}
