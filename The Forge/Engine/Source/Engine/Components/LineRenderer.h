#pragma once
#include "ComponentUtils.h"
#include "SpriteRenderer.h"

namespace Engine
{
    class LineRenderer final: public SpriteRenderer
    {
    public:
        LineRenderer();
        void CollectUniforms(ShaderUniformData& data) override;
        void Render(const ShaderUniformData& data) override;
        
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;

        // Editor Properties 
        void DrawDetails() override;
        
    private:
        void RenderLine(glm::vec2 pos, glm::vec2 dir, glm::vec2 size);
        
    private:
        glm::vec2 _start;
        glm::vec2 _end;

    public:
        inline void SetStartAndEnd(const glm::vec2 start, const glm::vec2 end) { _start = start; _end = end; }
        
    };

    REGISTER_COMPONENT(LineRenderer);
}
