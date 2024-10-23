#pragma once
#include "Data.h"

namespace Engine
{
	class InputManager
	{
	public:

		InputManager();
        ~InputManager();

        // Used to collect inputs pressed this frame, returns true if the while loop should continue.
        bool StartProcessInputs();

        void ClearInputBuffers();

        // Used to switch the input buffers.
        void EndProcessInputs();

        int GetKey(int keycode) const;
        int GetKeyDown(int keycode) const;

        int GetButton(uint32 button, Vector2D& mousePos) const;
        int GetButtonDown(uint32 button, Vector2D& mousePos) const;

	private:

        // Keyboard Inputs
        uint8* _keysLastFrame;
        uint8* _keysThisFrame;
        int _numKeys;

        // Mouse Inputs
        uint32 _buttonsLastFrame;
        uint32 _buttonsThisFrame;

	};
}
