#pragma once
#include "ResourceCounter.h"
#include "Engine/Components/ComponentUtils.h"

class ResourceCounter;

class ResourceManager : public Engine::Component
{
public:
    ResourceManager();
    void Start() override;
    void Deserialize(const json& data) override;
    nlohmann::json Serialize() override;

private:
    ResourceCounter* _resourceCounter;
    int _resourceCount = 0;

public:
    void GainResources(const int gain) { _resourceCount += gain; if (_resourceCounter) _resourceCounter->UpdateResourceCounter(_resourceCount); };
};

REGISTER_COMPONENT(ResourceManager);