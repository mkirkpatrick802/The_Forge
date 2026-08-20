#pragma once
#include "ByteStream.h"
#include "IShaderUniformProvider.h"
#include "json.hpp"
#include "shader.h"

namespace Engine
{
    // The stock shaders. A renderable with no shader draws *nothing* -- CollectUniforms
    // bails on an invalid shader -- so leaving these empty meant a component added in
    // the editor was silently invisible until someone dragged shader files onto it.
    // Each concrete renderable picks its pair in its constructor.
    inline const std::string SPRITE_VERTEX_SHADER   = "Assets/Engine Assets/Shaders/Sprite.vert";
    inline const std::string SPRITE_FRAGMENT_SHADER = "Assets/Engine Assets/Shaders/Sprite.frag";
    inline const std::string LINE_VERTEX_SHADER     = "Assets/Engine Assets/Shaders/Line.vert";
    inline const std::string LINE_FRAGMENT_SHADER   = "Assets/Engine Assets/Shaders/Line.frag";
    inline const std::string TEXT_VERTEX_SHADER     = "Assets/Engine Assets/Shaders/Text.vert";
    inline const std::string TEXT_FRAGMENT_SHADER   = "Assets/Engine Assets/Shaders/Text.frag";

    class IRenderable : public IShaderUniformProvider
    {
    public:

        IRenderable();
        ~IRenderable() override;
        
        // This should be called on activation
        void InitRenderable(GameObject* go);
        virtual void Deserialize(const nlohmann::json& data);
        virtual nlohmann::json Serialize();

        virtual void Write(NetCode::OutputByteStream& stream) const;
        virtual void Read(NetCode::InputByteStream& stream); 
        
        virtual void Render(const ShaderUniformData& data) = 0;
        void CollectUniforms(ShaderUniformData& data) override {};

        // Creates this renderable's GPU-side resources if they don't exist yet.
        // Called by the Renderer immediately before drawing, and never during load:
        // a headless build never renders, so it never needs a GL context. Overrides
        // must call the base first.
        virtual void EnsureResourcesResident();

    protected:
        void DrawRenderableSettings();

        // Drops GPU resources so they are rebuilt on the next draw. Call after
        // changing anything the resources are derived from (a shader or sprite path).
        // Overrides should release their own resources and call the base.
        virtual void InvalidateResources() { _resourcesResident = false; }

        // The shaders to use until a level or prefab says otherwise. Called from a
        // concrete renderable's constructor -- a renderable with no shader draws
        // nothing at all, so every one of them needs a working pair from the start.
        void SetDefaultShaders(const std::string& vertexPath, const std::string& fragmentPath)
        {
            _vertexShaderFilepath = vertexPath;
            _fragmentShaderFilepath = fragmentPath;
        }

    protected:
        Shader shader;

    private:
        unsigned int _quadVAO;
        bool _resourcesResident;

        // What the last compile attempt used. A failed compile must not be retried every
        // frame forever, but it must be retried once the paths change -- otherwise a
        // component that failed once stays broken until the process restarts.
        std::string _compiledVertexShader;
        std::string _compiledFragmentShader;
        bool _shaderCompileFailed = false;

        std::string _vertexShaderFilepath;
        std::string _fragmentShaderFilepath;
        
        int8_t _sortingLayer;
        bool _isHidden;
        GameObject* _gameObject;

    public:
        unsigned int GetVAO() const { return _quadVAO; }
        int8_t GetSortingLayer() const { return _sortingLayer; }
        bool IsHidden() const { return _isHidden; }
        void SetHidden(const bool hidden) { _isHidden = hidden; }
        GameObject* GetGameObject() const { return _gameObject; }
        
    };
}
