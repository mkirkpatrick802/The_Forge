#pragma once
#include "ComponentUtils.h"
#include "SpriteRenderer.h"

namespace Engine
{
    class LineRenderer : public SpriteRenderer
    {
    public:
        LineRenderer();

        void Render() override;
        
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;

    private:
        glm::vec2 _start;
        glm::vec2 _end;
    };

    REGISTER_COMPONENT(LineRenderer);
}
