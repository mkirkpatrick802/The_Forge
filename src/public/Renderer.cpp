//
// Created by mKirkpatrick on 1/22/2024.
//

#include <iostream>
#include <windows.h>
#include <winuser.h>
#include <algorithm>
#include "Renderer.h"
#include <GL/glew.h>

#include <SDL_opengl.h>

#include "SpriteRenderer.h"
#include "GameObject.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

Renderer::Renderer(): _context(nullptr), _window(nullptr)
{
	_fullScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	_fullScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}

void Renderer::CreateRenderer()
{
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    _window = SDL_CreateWindow("The Forge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _editorWidth, _editorHeight, window_flags);
    if (_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        assert(0);
    }

    _context = SDL_GL_CreateContext(_window);
    SDL_GL_MakeCurrent(_window, _context);
    SDL_GL_SetSwapInterval(1);

    glewInit();
}

void Renderer::CreateSpriteRenderer(GameObject* go, const json& data = nullptr) {

    SpriteRenderer* spriteRenderer = _spriteRendererPool.New(go);
    spriteRenderer->Init();

    if(data != nullptr)
        spriteRenderer->LoadData(data);

    go->AddComponent(spriteRenderer);

    auto pairPTR = new std::pair<int16, SpriteRenderer*>(spriteRenderer->_sortingLayer, spriteRenderer);
    _renderList.push_back(pairPTR);

    SortRenderList();
}

void Renderer::SortRenderList() {

    std::sort(_renderList.begin(), _renderList.end(), [](auto &left, auto &right){
        return left->first < right->first;
    });
}

Vector2D Renderer::ConvertScreenToWorld(const Vector2D screenPos) {
    Vector2D worldLocation = Vector2D(screenPos.x - (_editorWidth / 2), screenPos.y - (_editorHeight / 2));
    return {worldLocation.x / 2, -worldLocation.y / 2};
}

GameObject* Renderer::GetTopGameObject(std::vector<GameObject*> goList) {

    RenderListPair reversedList (_renderList.rbegin(), _renderList.rend());
    for (auto renderItem : reversedList)
        for (auto go : goList)
            if (renderItem->second->gameObject == go)
                return go;

    return nullptr;
}

void Renderer::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (!_renderList.empty())
		for (const auto &pair : _renderList)
            pair->second->DrawSprite();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(_window);
}


void Renderer::CleanUpSpriteRenderer(GameObject* go)
{
    if(SpriteRenderer* renderer = go->GetComponent<SpriteRenderer>())
    {
        for (auto &pair : _renderList) {
            if(pair->second == renderer)
                _renderList.erase(std::remove(_renderList.begin(), _renderList.end(), pair), _renderList.end());
        }

        _spriteRendererPool.Delete(renderer);
    }
}

void Renderer::CleanUp()
{

}
