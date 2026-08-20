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

        // Which image this draws. A PolygonCollider on the same object takes its shape
        // from here, which is what lets it be added with nothing to configure.
        const std::string& GetSpritePath() const { return _spritePath; }

        // A screen-space sprite bypasses the view matrix, so it has no world position
        // and world-space queries -- editor picking, for one -- must skip it.
        bool IsScreenSpace() const { return _isScreenSpace; }

    protected:
        void Render(const ShaderUniformData& data) override;
        void EnsureResourcesResident() override;
        void InvalidateResources() override;

    private:
        // The path is the durable state: it is what gets serialized and replicated,
        // and it survives whether or not the texture has been uploaded yet.
        std::string _spritePath;
        std::unique_ptr<Texture> _texture;
        glm::vec2 _size;
        bool _isScreenSpace;
        glm::vec2 _screenSpace;

    public:
        glm::vec2 GetScreenSpace() const { return _screenSpace; }
        void SetScreenSpace(const glm::vec2 screenSpace) { _screenSpace = screenSpace; }

    };

    REGISTER_COMPONENT(SpriteRenderer)
}