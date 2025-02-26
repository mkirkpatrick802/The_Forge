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

    protected:
        void DrawRenderableSettings();

    protected:
        Shader shader;
        
    private:
        unsigned int _quadVAO;
        
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
