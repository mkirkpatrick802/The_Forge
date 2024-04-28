#pragma once

#include <vector>

#include "GameData.h"
#include "ComponentPool.h"
#include "SpriteRenderer.h"
#include <SDL_render.h>

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

    GameObject* GetTopGameObject(const std::vector<GameObject*>& goList);

    void Render() const;

    void CleanUpSpriteRenderer(GameObject* go);
    void CleanUp();

    // Getters
    SDL_GLContext GetDefaultContext() const { return _context; }
    SDL_Window* GetDefaultWindow() const { return _window; }

    static Vector2D ConvertScreenToWorld(const Vector2D screenPos);
    static Vector2D ConvertWorldToScreen(const Vector2D worldPos);

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