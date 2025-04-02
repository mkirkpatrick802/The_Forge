#pragma once
#include "ComponentUtils.h"
#include "SpriteRenderer.h"

namespace Engine
{
    class LineRenderer final: public Component, public IRenderable
    {
    public:
        LineRenderer();

        // Move constructor
        LineRenderer(LineRenderer&& other) noexcept;
        
        // Move assignment operator
        LineRenderer& operator=(LineRenderer&& other) noexcept;
        
        void OnActivation() override;
        void Render(const ShaderUniformData& data) override;

        virtual void Deserialize(const json& data) override;
        virtual nlohmann::json Serialize() override;

        virtual void Write(NetCode::OutputByteStream& stream) const override;
        virtual void Read(NetCode::InputByteStream& stream) override;
        
        // Editor Properties 
        void DrawDetails() override;
        
    private:
        void RenderLine(glm::vec2 pos, glm::vec2 dir, glm::vec2 size);
        
    private:
        std::unique_ptr<Texture> _texture;
        glm::vec2 _size;
        
        glm::vec2 _start;
        glm::vec2 _end;

    public:
        inline void SetStartAndEnd(const glm::vec2 start, const glm::vec2 end) { _start = start; _end = end; }
        
    };

    REGISTER_COMPONENT(LineRenderer);
}
