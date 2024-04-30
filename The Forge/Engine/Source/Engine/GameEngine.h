#pragma once

namespace Engine
{
	class Renderer;
	class InputManager;
	class GameEngine
	{
	public:
		static GameEngine* GetInstance();

		void StartGamePlayLoop() const;

		void CleanUp();

	private:

		GameEngine();

		static GameEngine* _instance;

		Renderer* _renderer;
		InputManager* _inputManager;
	};

}
