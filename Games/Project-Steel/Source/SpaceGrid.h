#pragma once
#include "Engine/Components/SpriteRenderer.h"

class SpaceGrid : public Engine::SpriteRenderer
{
public:
    SpaceGrid();
    void OnActivation() override;
    void Render(const Engine::ShaderUniformData& data) override;
    void DrawDetails() override;
    
private:
    void RenderTile(glm::vec2 pos);

private:
    std::vector<Engine::Texture> _sprites;
    glm::vec2 _gridSize;
};

REGISTER_COMPONENT(SpaceGrid);
