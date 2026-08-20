#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <SDL_scancode.h>

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
        int GetButton(uint32_t button) const;
			
        int GetButtonDown(uint32_t button, glm::vec2& mousePos) const;
        int GetButtonDown(uint32_t button) const;

		// Mouse Position in Screen Space
		void GetMousePos(glm::vec2& mousePos) const;
		int32_t GetMouseWheelDelta() const;

		// --- consumption ---
		//
		// Marks input as already dealt with, so everything that reads it later this frame
		// is told it did not happen. This is what stops a click on a menu button from
		// also firing in the world underneath it.
		//
		// Ordering is what makes it work: UIRoot::Update runs before
		// ComponentManager::UpdateComponents, so the UI has already claimed whatever it
		// wanted by the time gameplay looks. Cleared every frame in EndProcessInputs,
		// so a consumer has to claim it again next frame -- there is no sticky grab.
		//
		// Deliberately separate from ImGui's WantCaptureMouse, which is handled in
		// StartProcessInputs instead: ImGui consumes events before anyone sees them,
		// whereas this consumes state after the UI has had its turn.
		void ConsumeMouseButtons();
		void ConsumeKey(int keycode);

	private:
        // Keyboard Inputs
        uint8_t* _keysLastFrame;
        uint8_t* _keysThisFrame;

		// One flag per scancode, cleared each frame. Declared alongside the buffers above
		// for years without ever being allocated -- consumption is what finally uses it.
		bool* _keysConsumedThisFrame;
        int _numKeys;

        // Mouse Inputs
        uint32_t _buttonsLastFrame;
        uint32_t _buttonsThisFrame;

		// A mask of buttons claimed this frame, in the same bit layout as the state
		// above -- so masking one against the other is the whole implementation.
		uint32_t _buttonsConsumedThisFrame;
		int32_t _mouseWheelDelta;

	};
}
