#include "ResourceManager.h"

#include "Engine/JsonKeywords.h"

ResourceManager::ResourceManager(): _resourceCounter(nullptr)
{
}

void ResourceManager::Start()
{
    _resourceCounter = gameObject->GetComponent<ResourceCounter>();
}
