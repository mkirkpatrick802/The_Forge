#pragma once
#include "Component.h"
#include "ComponentUtils.h"
#include "Engine/Rendering/IRenderable.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    class SpriteRenderer : public Component, public IRenderable
    {
        friend class Renderer;

    public:
        SpriteRenderer();

        void OnActivation() override;
        void CollectUniforms(ShaderUniformData& data) override;
        
        virtual void Deserialize(const json& data) override;
        virtual nlohmann::json Serialize() override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;
        
        virtual void DrawDetails() override;
        
        glm::vec2 GetSize() const { return _size; }

    protected:
        void Render(const ShaderUniformData& data) override;

    protected:
        std::unique_ptr<Texture> _texture;
        glm::vec2 _size;

    };

    REGISTER_COMPONENT(SpriteRenderer)
}