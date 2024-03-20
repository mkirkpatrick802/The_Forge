//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_SPRITERENDERER_H
#define THE_FORGE_SPRITERENDERER_H

#include "GameData.h"
#include "Component.h"
#include <SDL_render.h>
#include "Shader.h"

class Renderer;

typedef SDL_Texture Texture;

class SpriteRenderer : public Component {

    friend class Renderer;

public:

    const static uint32 ComponentID = SPRITE_RENDERER;

    SpriteRenderer();
    virtual void LoadData(const json& data) override;

private:

    void Init();
    void DrawSprite();

private:

    Shader       _shader;
    unsigned int _quadVAO;

    //Texture2D     _texture;
    int16        _sortingLayer = 0;
    Vector2D     _size = Vector2D(16);

public:
	Vector2D GetSize() const {return _size;}
};

#endif //THE_FORGE_SPRITERENDERER_H
