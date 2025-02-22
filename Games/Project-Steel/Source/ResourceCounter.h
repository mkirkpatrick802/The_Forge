#pragma once
#include "Engine/Components/ComponentUtils.h"
#include "Engine/Components/TextRenderer.h"

class ResourceCounter final : public Engine::TextRenderer
{
public:
    void UpdateResourceCounter(int resourceCount);
    void Deserialize(const json& data) override;
    nlohmann::json Serialize() override;
};

REGISTER_COMPONENT(ResourceCounter)
