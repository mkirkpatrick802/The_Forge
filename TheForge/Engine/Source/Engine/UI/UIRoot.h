#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "UICanvas.h"

namespace Engine
{
    class UIScreen;

    // The stack of active UI screens, and the engine's single entry point into the UI
    // layer.
    //
    // Static in the same shape as UIManager, which is the existing precedent for "a
    // presentation subsystem the renderer drives". The difference is what it draws:
    // UIManager is Dear ImGui and stays topmost, this is the game's own screen-space UI
    // and sits between the scene and ImGui.
    //
    // Nothing here may be called on a headless build. There is no GL context on a
    // dedicated server, so a UI screen is not merely idle there -- it must not exist. The
    // guards below make that a no-op rather than a crash, but the caller should still not
    // be reaching for it.
    class UIRoot
    {
        friend class Renderer;
        friend class GameEngine;

    public:
        // Takes ownership and calls OnEnter. Deferred to the top of the next Update, so
        // a screen may push another from inside a button callback without invalidating
        // the stack that callback is being iterated from.
        static void Push(std::unique_ptr<UIScreen> screen);

        // Pops the top screen. Also deferred, for the same reason: "Back" is a button on
        // the very screen being removed.
        static void Pop();

        // Pops everything, then pushes. The usual transition between two peer screens --
        // main menu to loading screen -- where "back" is not meaningful.
        static void Replace(std::unique_ptr<UIScreen> screen);

        static void PopAll();

        // The top screen, or nullptr if the stack is empty. Does not see screens that
        // are still queued.
        static UIScreen* GetTop();

        // Whether a screen by that name is on the stack, including queued pushes -- so
        // that "is the loading screen already up" is answerable in the same frame it was
        // asked for.
        static bool IsOpen(const std::string& name);

        static bool IsEmpty();

        // True while the pointer is over UI, or a screen is blocking input outright.
        // Read by InputManager so a click on a button does not also land in the world.
        static bool IsPointerOverUI() { return _pointerOverUI; }

        // The canvas that belongs to the world rather than to a screen.
        //
        // Where level-authored UI lives: a UIWidget component registers its element here
        // on activation and takes it off on deactivation, so a HUD or a menu laid out in
        // the editor draws without any screen having to know it exists. Drawn beneath
        // every screen, because a pause menu or a loading screen must cover the HUD.
        //
        // Cleared when the level goes, which UIWidget handles by unregistering itself --
        // the canvas only ever holds borrowed pointers, and the GameObject owns the
        // component that owns the element.
        static UICanvas& GetWorldCanvas();

        // Window pixels to canvas units. The canvas is authored at
        // Engine::ReferenceResolution, so this is what turns a raw SDL mouse position
        // into something a UIRect can be tested against.
        static glm::vec2 WindowToUI(const glm::vec2& windowPosition);

    private:
        static void Update(float deltaTime);
        static void UpdateWorldCanvas(float deltaTime, const glm::vec2& pointer, bool pointerDown);
        static void Render();
        static void CleanUp();

        // Applies queued pushes and pops. Called at the top of Update and again before
        // Render, so a screen pushed from a callback is visible on the very next frame
        // rather than the one after.
        static void Flush();

    private:
        static UICanvas _worldCanvas;
        static std::vector<std::unique_ptr<UIScreen>> _screens;
        static std::vector<std::unique_ptr<UIScreen>> _pendingPushes;
        static int _pendingPops;
        static bool _pointerOverUI;
    };
}
