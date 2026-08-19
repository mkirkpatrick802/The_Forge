#pragma once
#include "Engine/Components/ComponentUtils.h"
#include "Engine/Components/TextRenderer.h"

class ResourceCounter final : public Engine::TextRenderer
{
public:
    void Render(const Engine::ShaderUniformData& data) override;
    void UpdateResourceCounter(int resourceCount);

    // Deserialize is deliberately *not* overridden. It used to build the Font and
    // register with the renderer right here, which is GPU work at load time -- it
    // crashed a dedicated server the first time one ever spawned a pawn.
    // TextRenderer::Deserialize invalidates instead, and the font is built on the
    // first draw via EnsureResourcesResident.
    nlohmann::json Serialize() override;
};

REGISTER_COMPONENT(ResourceCounter)
