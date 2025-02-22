#include "TextRenderer.h"

#include "Engine/JsonKeywords.h"

Engine::TextRenderer::TextRenderer()
{
    
}

void Engine::TextRenderer::Deserialize(const json& data)
{
    
}

nlohmann::json Engine::TextRenderer::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<TextRenderer>();
    return data;
}
