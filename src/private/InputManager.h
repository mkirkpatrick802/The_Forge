//
// Created by mKirkpatrick on 1/23/2024.
//

#ifndef THE_FORGE_INPUTMANAGER_H
#define THE_FORGE_INPUTMANAGER_H

#include "Renderer.h"

class InputManager {
public:

    InputManager();

    // Used to collect inputs pressed this frame, returns true if the while loop should continue.
    bool StartProcessInputs(Renderer& renderer, bool uiEnabled = true);

    // Used to switch the input buffers.
    void EndProcessInputs();

    int GetKey(const int keycode);
    int GetKeyDown(const int keycode);

    int GetButton(const int button, Vector2D& mousePos);
    int GetButtonDown(const int button, Vector2D& mousePos);

    void CleanUp();

protected:
private:

    // Keyboard Inputs
    uint8* _keysLastFrame;
    uint8* _keysThisFrame;
    int _numKeys;

    // Mouse Inputs
    uint32 _buttonsLastFrame;
    uint32 _buttonsThisFrame;
};


#endif //THE_FORGE_INPUTMANAGER_H
