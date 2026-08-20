#include "UIRoot.h"

#include <SDL_mouse.h>

#include "glad/glad.h"

#include "UIResources.h"
#include "UIScreen.h"
#include "Engine/GameEngine.h"
#include "Engine/InputManager.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"
#include "Engine/Rendering/Renderer.h"

Engine::UICanvas Engine::UIRoot::_worldCanvas;
std::vector<std::unique_ptr<Engine::UIScreen>> Engine::UIRoot::_screens;
std::vector<std::unique_ptr<Engine::UIScreen>> Engine::UIRoot::_pendingPushes;
int Engine::UIRoot::_pendingPops = 0;
bool Engine::UIRoot::_pointerOverUI = false;

void Engine::UIRoot::Push(std::unique_ptr<UIScreen> screen)
{
    if (screen == nullptr) return;

    // A dedicated server has no window, no GL context and no player to show anything to.
    // Dropping the screen here rather than asserting keeps game code free of "unless
    // headless" checks around every menu transition.
    if (GetLaunchOptions().headless) return;

    _pendingPushes.push_back(std::move(screen));
}

void Engine::UIRoot::Pop()
{
    ++_pendingPops;
}

void Engine::UIRoot::Replace(std::unique_ptr<UIScreen> screen)
{
    // Enough pops to clear what is standing now. Counted rather than cleared outright
    // because the pops are applied in Flush, alongside any that were already queued.
    _pendingPops = static_cast<int>(_screens.size());
    Push(std::move(screen));
}

void Engine::UIRoot::PopAll()
{
    _pendingPops = static_cast<int>(_screens.size());
    _pendingPushes.clear();
}

Engine::UICanvas& Engine::UIRoot::GetWorldCanvas()
{
    return _worldCanvas;
}

Engine::UIScreen* Engine::UIRoot::GetTop()
{
    return _screens.empty() ? nullptr : _screens.back().get();
}

bool Engine::UIRoot::IsEmpty()
{
    return _screens.empty() && _pendingPushes.empty();
}

bool Engine::UIRoot::IsOpen(const std::string& name)
{
    for (const auto& screen : _screens)
        if (screen->GetName() == name) return true;

    // Queued pushes count. Otherwise code that asks "is the loading screen up?" in the
    // same frame it asked for one would be told no and open a second.
    for (const auto& screen : _pendingPushes)
        if (screen->GetName() == name) return true;

    return false;
}

glm::vec2 Engine::UIRoot::WindowToUI(const glm::vec2& windowPosition)
{
    const glm::vec2 windowSize = GetAppWindowSize();
    if (windowSize.x <= 0.0f || windowSize.y <= 0.0f) return glm::vec2(0.0f);

    const UIScreen* top = GetTop();
    const glm::vec2 canvasSize = top != nullptr ? top->Canvas().GetCanvasSize() : ReferenceResolution;

    return windowPosition / windowSize * canvasSize;
}

void Engine::UIRoot::Flush()
{
    while (_pendingPops > 0 && !_screens.empty())
    {
        _screens.back()->OnExit();
        _screens.pop_back();
        --_pendingPops;
    }

    // Anything left over is a pop against an empty stack. Dropped rather than carried,
    // so a stray Pop() cannot silently eat the next screen that gets pushed.
    _pendingPops = 0;

    if (_pendingPushes.empty()) return;

    // Moved out first. OnEnter is allowed to push another screen, and that push must
    // land in a queue this loop is not currently walking.
    std::vector<std::unique_ptr<UIScreen>> pushes = std::move(_pendingPushes);
    _pendingPushes.clear();

    for (auto& screen : pushes)
    {
        _screens.push_back(std::move(screen));
        _screens.back()->OnEnter();
    }
}

void Engine::UIRoot::UpdateWorldCanvas(const float deltaTime, const glm::vec2& pointer, const bool pointerDown)
{
    if (_worldCanvas.Update(deltaTime, pointer, pointerDown))
        _pointerOverUI = true;
}

void Engine::UIRoot::Update(const float deltaTime)
{
    Flush();

    _pointerOverUI = false;

    glm::vec2 mouseWindowPos(0.0f);
    GetInputManager().GetMousePos(mouseWindowPos);

    const glm::vec2 pointer = WindowToUI(mouseWindowPos);
    const bool pointerDown = GetInputManager().GetButton(SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    // Top-down, stopping at the first screen that is not input-transparent. A pause menu
    // over the game must not let a click through to the world, and a settings page over
    // a pause menu must not let one through to Resume.
    bool reachedWorld = true;

    for (auto screen = _screens.rbegin(); screen != _screens.rend(); ++screen)
    {
        if ((*screen)->Canvas().Update(deltaTime, pointer, pointerDown))
            _pointerOverUI = true;

        (*screen)->Update(deltaTime);

        if (!(*screen)->IsInputTransparent())
        {
            // A screen that blocks input stops the walk here, and the world canvas is
            // below every screen -- so level-authored UI is unreachable while a menu is
            // up, which is the whole point of a menu being modal.
            reachedWorld = false;
            break;
        }
    }

    if (reachedWorld)
        UpdateWorldCanvas(deltaTime, pointer, pointerDown);

    // Claimed here, not inside the canvas, so it happens once for the whole stack and
    // only after every screen that was going to react has done so. Everything that reads
    // input later this frame -- ComponentManager::UpdateComponents, and the game mode
    // after it -- is told the click did not happen.
    if (_pointerOverUI)
        GetInputManager().ConsumeMouseButtons();

    // Again, so a screen pushed or popped from a button callback this frame is on the
    // stack before Render walks it.
    Flush();
}

void Engine::UIRoot::Render()
{
    if (_screens.empty() && _worldCanvas.IsEmpty()) return;

    // Set explicitly rather than inherited. The scene pass sets the viewport to the
    // scene framebuffer's size, and that framebuffer's resize is deferred by a frame --
    // so on the frame after a window resize the viewport is still the old size, and UI
    // drawn under it would be letterboxed against the new backbuffer.
    const glm::vec2 windowSize = GetAppWindowSize();
    if (windowSize.x <= 0.0f || windowSize.y <= 0.0f) return;

    glViewport(0, 0, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));

    // The UI is authored with transparency throughout -- a font atlas is pure alpha, and
    // panels are routinely translucent -- so blending has to be on regardless of what the
    // last pass left enabled.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (_screens.empty())
    {
        _worldCanvas.Render();
        return;
    }

    // Find the lowest screen that has to be drawn: everything from the last opaque one
    // upwards. A pause menu is transparent and wants what is below it drawn; a main menu
    // is not, and would only be paying to draw something nobody can see.
    size_t first = _screens.size() - 1;
    while (first > 0 && _screens[first]->IsTransparent())
        --first;

    // The world canvas only draws when nothing opaque is standing over it.
    if (first == 0 && _screens[0]->IsTransparent())
        _worldCanvas.Render();

    for (size_t i = first; i < _screens.size(); ++i)
        _screens[i]->Canvas().Render();
}

void Engine::UIRoot::CleanUp()
{
    // Through OnExit, not a bare clear. A screen that installed a log mirror or grabbed
    // input has to be given the chance to put it back before the process goes down.
    while (!_screens.empty())
    {
        _screens.back()->OnExit();
        _screens.pop_back();
    }

    _pendingPushes.clear();
    _pendingPops = 0;

    // Borrowed pointers only -- the UIWidget components that own these elements are
    // torn down with their level. Dropping the references stops a later draw walking
    // into freed memory if the level outlives this call by any path.
    _worldCanvas.Clear();

    // While the GL context is still alive -- the renderer calls this from its own
    // destructor, before SDL_GL_DeleteContext.
    UIResources::CleanUp();
}
