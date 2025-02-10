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

    // Mouse State Init
    int X, Y;
    const uint32_t currentState = SDL_GetMouseState(&X, &Y);
    _buttonsLastFrame = currentState;
    _buttonsThisFrame = currentState;
}

bool Engine::InputManager::StartProcessInputs()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
    	ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(GetAppWindow()))
            return false;
    }

    //Current Inputs Pressed
    int X, Y;
    memcpy(_keysThisFrame, SDL_GetKeyboardState(nullptr), _numKeys);
    _buttonsThisFrame = SDL_GetMouseState(&X, &Y);

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
}

int Engine::InputManager::GetKey(const int keycode) const
{
    if (keycode < _numKeys)
        return _keysThisFrame[keycode];

    return false;
}

int Engine::InputManager::GetKeyDown(const int keycode) const
{

    if (keycode < _numKeys)
        return !_keysLastFrame[keycode] && _keysThisFrame[keycode];

    return false;
}

int Engine::InputManager::GetButton(const uint32_t button, glm::vec2& mousePos) const
{
    int X, Y;
    SDL_GetMouseState(&X, &Y);
    mousePos = glm::vec2((float)X, (float)Y);
    return _buttonsThisFrame & button;
}

int Engine::InputManager::GetButtonDown(const uint32_t button, glm::vec2& mousePos) const
{
    int X, Y;
    SDL_GetMouseState(&X, &Y);
    mousePos = glm::vec2((float)X, (float)Y);
    return _buttonsThisFrame & button && (!_buttonsLastFrame) & button;
}

int Engine::InputManager::GetButtonDown(uint32_t button) const
{
    return _buttonsThisFrame & button && (!_buttonsLastFrame) & button;
}

Engine::InputManager::~InputManager()
{
    delete[] _keysLastFrame;
    _keysLastFrame = nullptr;

    delete[] _keysThisFrame;
    _keysThisFrame = nullptr;
}
