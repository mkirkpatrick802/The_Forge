#pragma once
#include "ByteStream.h"
#include "IShaderUniformProvider.h"
#include "json.hpp"
#include "shader.h"

namespace Engine
{
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

    protected:
        Shader shader;
        
    private:
        unsigned int _quadVAO;
        bool _resourcesResident;

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
