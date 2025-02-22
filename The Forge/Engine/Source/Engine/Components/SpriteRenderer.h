#pragma once
#include "Component.h"
#include "ComponentUtils.h"
#include "Engine/Rendering/IShaderUniformProvider.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    class SpriteRenderer : public Component, public IShaderUniformProvider
    {
        friend class Renderer;

    public:
        SpriteRenderer();
        ~SpriteRenderer() override;

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
        unsigned int _quadVAO;
        Shader _shader;
        
        std::string _vertexShaderFilepath;
        std::string _fragmentShaderFilepath;
        
        std::unique_ptr<Texture> _texture;
        
        glm::vec2 _size;
        bool _hidden;

    public:
        inline void SetHidden(const bool hidden) { _hidden = hidden; }

    };

    REGISTER_COMPONENT(SpriteRenderer)
}