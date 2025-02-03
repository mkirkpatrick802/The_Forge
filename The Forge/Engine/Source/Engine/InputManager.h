#pragma once
#include <cstdint>
#include <glm/vec2.hpp>

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

        int GetButton(uint32_t button, glm::vec2& mousePos) const;
        int GetButtonDown(uint32_t button, glm::vec2& mousePos) const;
        int GetButtonDown(uint32_t button) const;

	private:

        // Keyboard Inputs
        uint8_t* _keysLastFrame;
        uint8_t* _keysThisFrame;
        int _numKeys;

        // Mouse Inputs
        uint32_t _buttonsLastFrame;
        uint32_t _buttonsThisFrame;

	};
}
