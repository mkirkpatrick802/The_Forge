#pragma once
#include <random>

#include "Engine/Components/SpriteRenderer.h"

class SpaceGrid : public Engine::Component, public Engine::IRenderable
{
public:
    SpaceGrid();
    void OnActivation() override;
    void Render(const Engine::ShaderUniformData& data) override;

    void Deserialize(const json& data) override;
    nlohmann::json Serialize() override;
    void DrawDetails() override;
    
private:
    void InitTiles();
    void RenderTile(const Engine::Texture* sprite, glm::vec2 pos);

public:
    void CollectUniforms(Engine::ShaderUniformData& data) override {}

private:
    std::mt19937 gen;
    
    std::vector<std::pair<int, std::unique_ptr<Engine::Texture>>> _sprites;
    std::vector<std::pair<glm::vec2, const Engine::Texture*>> _tiles;
    glm::vec2 _gridSize;
};

REGISTER_COMPONENT(SpaceGrid);
