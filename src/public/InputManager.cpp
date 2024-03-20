//
// Created by mKirkpatrick on 1/23/2024.
//

#include "InputManager.h"

#include <SDL_events.h>
#include <SDL_mouse.h>
#include <backends/imgui_impl_sdl2.h>

InputManager::InputManager()
{

    // Keyboard State Init
    const uint8* currentKeys = SDL_GetKeyboardState(&_numKeys);
    _keysLastFrame = new uint8[_numKeys];
    _keysThisFrame = new uint8[_numKeys];

    memcpy(_keysLastFrame, currentKeys, _numKeys);
    memcpy(_keysThisFrame, currentKeys, _numKeys);

    // Mouse State Init
    int X, Y;
    const uint32 currentState = SDL_GetMouseState(&X, &Y);
    _buttonsLastFrame = currentState;
    _buttonsThisFrame = currentState;
}

bool InputManager::StartProcessInputs(Renderer& renderer, bool uiEnabled) {

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if(uiEnabled)
            ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(renderer.GetDefaultWindow()))
            return false;

        if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_ESCAPE)
                return false;
        }
    }

    //Current Inputs Pressed
    int X,Y;
    memcpy(_keysThisFrame, SDL_GetKeyboardState(nullptr), _numKeys);
    _buttonsThisFrame = SDL_GetMouseState(&X, &Y);

    return true;
}

void InputManager::EndProcessInputs() {

    {
        // Keyboard Buffer Switch
        uint8* temp = _keysThisFrame;
        _keysThisFrame = _keysLastFrame;
        _keysLastFrame = temp;
    }

    {
        // Mouse Buffer Switch
        uint32 temp = _buttonsThisFrame;
        _buttonsThisFrame = _buttonsLastFrame;
        _buttonsLastFrame = temp;
    }
}

// Returns 1 if input is pressed, and 0 if not
int InputManager::GetKey(const int keycode) {

    if (keycode < _numKeys)
        return _keysThisFrame[keycode];

    assert(0 && "Invalid Input");
}

// Returns 1 if input is pressed, and 0 if not
int InputManager::GetKeyDown(const int keycode) {

    if (keycode < _numKeys){

        return !_keysLastFrame[keycode] && _keysThisFrame[keycode];
    }

    assert(0 && "Invalid Input");
}

int InputManager::GetButton(const int button, Vector2D &mousePos) {

    int X, Y;
    SDL_GetMouseState(&X,&Y);
    mousePos = Vector2D((float)X, (float)Y);
    return _buttonsThisFrame & button;
}

int InputManager::GetButtonDown(const int button, Vector2D &mousePos) {

    int X, Y;
    SDL_GetMouseState(&X,&Y);
    mousePos = Vector2D((float)X, (float)Y);
    return (_buttonsThisFrame & button) && (!_buttonsLastFrame & button);
}

void InputManager::CleanUp() {

}
