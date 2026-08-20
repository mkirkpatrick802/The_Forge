#include "InputManager.h"
#include <SDL_events.h>
#include <SDL_mouse.h>

#include "System.h"
#include "backends/imgui_impl_sdl2.h"

Engine::InputManager::InputManager()
{
    // Keyboard State Init
    const uint8_t* currentKeys = SDL_GetKeyboardState(&_numKeys);
    _keysLastFrame = new uint8_t[_numKeys];
    _keysThisFrame = new uint8_t[_numKeys];

    memcpy(_keysLastFrame, currentKeys, _numKeys);
    memcpy(_keysThisFrame, currentKeys, _numKeys);

    _keysConsumedThisFrame = new bool[_numKeys]();

    // Mouse State Init
    int X, Y;
    const uint32_t currentState = SDL_GetMouseState(&X, &Y);
    _buttonsLastFrame = currentState;
    _buttonsThisFrame = currentState;

    _buttonsConsumedThisFrame = 0;
    _mouseWheelDelta = 0;
}

bool Engine::InputManager::StartProcessInputs()
{
    SDL_Event event;
    _mouseWheelDelta = 0;

    const ImGuiIO& io = ImGui::GetIO();
    bool wantKeyboard = io.WantCaptureKeyboard;
    bool wantMouse = io.WantCaptureMouse;
    
    while (SDL_PollEvent(&event))
    {
    	ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
        {
            APPLICATION_CLOSING = true;
            return false;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(GetAppWindow()))
        {
            APPLICATION_CLOSING = true;
            return false;
        }

        if(event.type == SDL_MOUSEWHEEL)
            _mouseWheelDelta = event.wheel.y;
    }

    // Zeroed rather than left alone while ImGui has focus.
    //
    // Skipping the copy freezes the buffer instead of clearing it, so a key held at the
    // moment the chat window opened stayed "held" for as long as it had focus -- which
    // reads in game as the player walking into a wall until they click away.
    if (wantKeyboard)
        memset(_keysThisFrame, 0, _numKeys);
    else
        memcpy(_keysThisFrame, SDL_GetKeyboardState(nullptr), _numKeys);

    int X, Y;
    const uint32_t mouseState = SDL_GetMouseState(&X, &Y);

    // wantMouse was computed here and then never used, so a click on an ImGui button
    // was delivered to the game as well -- pressing Play in a menu also fired whatever
    // the world had bound to left click.
    _buttonsThisFrame = wantMouse ? 0 : mouseState;

    return true;
}

void Engine::InputManager::ClearInputBuffers()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    ImGui_ImplSDL2_ProcessEvent(&event);

    EndProcessInputs();
}

void Engine::InputManager::EndProcessInputs()
{
    {
        // Keyboard Buffer Switch
        uint8_t* temp = _keysThisFrame;
        _keysThisFrame = _keysLastFrame;
        _keysLastFrame = temp;
    }

    {
        // Mouse Buffer Switch
        uint32_t temp = _buttonsThisFrame;
        _buttonsThisFrame = _buttonsLastFrame;
        _buttonsLastFrame = temp;
    }

    // Consumption lasts exactly one frame. A claim that outlived its frame would be a
    // grab, and nothing here has any way to release one.
    _buttonsConsumedThisFrame = 0;
    memset(_keysConsumedThisFrame, 0, _numKeys);
}

void Engine::InputManager::ConsumeMouseButtons()
{
    // Every button, not just the one that was acted on. A UI element under the cursor
    // owns the pointer for the frame; letting a right-click through to the world while
    // the left was claimed is the sort of split that is impossible to reason about.
    _buttonsConsumedThisFrame = 0xFFFFFFFF;
}

void Engine::InputManager::ConsumeKey(const int keycode)
{
    if (keycode >= 0 && keycode < _numKeys)
        _keysConsumedThisFrame[keycode] = true;
}

int Engine::InputManager::GetKey(const int keycode) const
{
    if (keycode < 0 || keycode >= _numKeys) return false;
    if (_keysConsumedThisFrame[keycode]) return false;

    return _keysThisFrame[keycode];
}

int Engine::InputManager::GetKeyDown(const int keycode) const
{
    if (keycode < 0 || keycode >= _numKeys) return false;
    if (_keysConsumedThisFrame[keycode]) return false;

    return !_keysLastFrame[keycode] && _keysThisFrame[keycode];
}

int Engine::InputManager::GetButton(const uint32_t button, glm::vec2& mousePos) const
{
    int X, Y;
    SDL_GetMouseState(&X, &Y);
    mousePos = glm::vec2((float)X, (float)Y);
    return GetButton(button);
}

int Engine::InputManager::GetButton(const uint32_t button) const
{
    return _buttonsThisFrame & button & ~_buttonsConsumedThisFrame;
}

int Engine::InputManager::GetButtonDown(const uint32_t button, glm::vec2& mousePos) const
{
    int X, Y;
    SDL_GetMouseState(&X, &Y);
    mousePos = glm::vec2((float)X, (float)Y);
    return GetButtonDown(button);
}

int Engine::InputManager::GetButtonDown(const uint32_t button) const
{
    // Was: _buttonsThisFrame & button && (!_buttonsLastFrame) & button
    //
    // & binds tighter than &&, so the right-hand side parsed as (!_buttonsLastFrame) &
    // button -- a logical negation of the whole mask, reduced to 0 or 1, and then masked.
    // It happened to work for the left button, whose mask is 1, and silently never
    // fired for the middle or right button, whose masks are 2 and 4. It also demanded
    // that *no* button at all was held last frame, so left-click while holding right
    // registered as nothing.
    if (_buttonsConsumedThisFrame & button) return false;

    return (_buttonsThisFrame & button) && !(_buttonsLastFrame & button);
}

void Engine::InputManager::GetMousePos(glm::vec2& mousePos) const
{
    int X, Y;
    SDL_GetMouseState(&X, &Y);
    mousePos = glm::vec2((float)X, (float)Y);
}

int32_t Engine::InputManager::GetMouseWheelDelta() const
{
    return _mouseWheelDelta;
}

Engine::InputManager::~InputManager()
{
    delete[] _keysLastFrame;
    _keysLastFrame = nullptr;

    delete[] _keysThisFrame;
    _keysThisFrame = nullptr;

    delete[] _keysConsumedThisFrame;
    _keysConsumedThisFrame = nullptr;
}
