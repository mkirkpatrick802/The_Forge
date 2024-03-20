//
// Created by mKirkpatrick on 1/22/2024.
//

#ifndef THE_FORGE_RENDERER_H
#define THE_FORGE_RENDERER_H

#include <vector>

#include "GameData.h"
#include "ComponentPool.h"
#include "SpriteRenderer.h"

struct SDL_Renderer;
struct SDL_Window;
class SpriteRenderer;

typedef std::vector<std::pair<int16, SpriteRenderer*>*> RenderListPair;

class Renderer {
public:

    Renderer();

    void CreateRenderer();

    void CreateSpriteRenderer(GameObject* go, const json& data);
    void SortRenderList();

    GameObject* GetTopGameObject(std::vector<GameObject*> goList);

    void Render();

    void CleanUpSpriteRenderer(GameObject* go);
    void CleanUp();

    // Getters
    SDL_GLContext GetDefaultContext() { return _context; }
    SDL_Window* GetDefaultWindow() {return _window;}

    static Vector2D ConvertScreenToWorld(const Vector2D screenPos);

private:

    // Editor Size
    static const uint32 _editorWidth = 1280;
    static const uint32 _editorHeight = 720;

    static const uint32 _logicalWidth = 640;
    static const uint32 _logicalHeight = 360;

    // Fullscreen Size
    uint32 _fullScreenWidth;
    uint32 _fullScreenHeight;

    RenderListPair _renderList;

    // Game & Editor
    SDL_GLContext _context;
    SDL_Window* _window;

    ComponentPool<SpriteRenderer> _spriteRendererPool;
};


#endif //THE_FORGE_RENDERER_H
