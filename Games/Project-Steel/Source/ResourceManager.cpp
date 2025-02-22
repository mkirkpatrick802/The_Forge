#include "ResourceManager.h"

#include "Engine/JsonKeywords.h"

ResourceManager::ResourceManager(): _resourceCounter(nullptr)
{
}

void ResourceManager::Start()
{
    _resourceCounter = gameObject->GetComponent<ResourceCounter>();
}

void ResourceManager::Deserialize(const json& data)
{
    
}

nlohmann::json ResourceManager::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = Engine::GetComponentRegistry().GetComponentID<ResourceManager>();
    return data;
}
