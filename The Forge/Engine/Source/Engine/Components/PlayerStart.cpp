#include "PlayerStart.h"

#include "Engine/JsonKeywords.h"

void Engine::PlayerStart::Deserialize(const json& data)
{
    
}

nlohmann::json Engine::PlayerStart::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<PlayerStart>();
    
    return data;
}
