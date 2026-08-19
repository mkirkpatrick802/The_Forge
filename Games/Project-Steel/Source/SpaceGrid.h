#pragma once
#include <random>
#include <string>
#include <unordered_map>
#include "Engine/Components/SpriteRenderer.h"

class SpaceGrid : public Engine::Component, public Engine::IRenderable
{
public:
    SpaceGrid();
    ~SpaceGrid() override;
    
    void OnActivation() override;
    void Render(const Engine::ShaderUniformData& data) override;

    void DrawDetails() override;
    void Deserialize(const json& data) override;
    nlohmann::json Serialize() override;

    void Write(NetCode::OutputByteStream& stream) const override;
    void Read(NetCode::InputByteStream& stream) override;
    
private:
    void InitTiles();
    void RenderTile(const Engine::Texture* sprite, glm::vec2 pos);

    // Texture for a sprite path, uploaded on first use. Render path only -- this is
    // the one place here that needs a GL context.
    const Engine::Texture* GetTileTexture(const std::string& path);

public:
    void CollectUniforms(Engine::ShaderUniformData& data) override {}

private:
    // A sprite in the weighted palette. Path and size come from the asset's import
    // metadata at load time, so tile layout can be computed without opening or
    // uploading the image.
    struct SpriteAsset
    {
        std::string path;
        glm::vec2 size = glm::vec2(0.0f);
    };

    std::mt19937 _gen;

    const int _tileSize = 256;
    std::vector<std::pair<int, SpriteAsset>> _sprites;      // weight, sprite
    std::vector<std::pair<glm::vec2, std::string>> _tiles;  // pos, sprite path

    std::unordered_map<std::string, std::unique_ptr<Engine::Texture>> _textureCache;

    glm::vec2 _gridSize;
};

REGISTER_COMPONENT(SpaceGrid);
