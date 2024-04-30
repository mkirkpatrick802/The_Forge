#pragma once
#include "System.h"

namespace Engine
{
	class Renderer;
	class InputManager;
	class GameEngine
	{
	public:
		static GameEngine* GetInstance();

		void StartGamePlayLoop();

		void CleanUp();

	private:

		static void Init();
		GameEngine();

		static GameEngine* _instance;

		Renderer* _renderer;
		InputManager* _inputManager;
	};

}
