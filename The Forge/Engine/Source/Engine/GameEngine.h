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
		static void CleanUp();

	private:

		GameEngine();
		~GameEngine();

	private:

		static GameEngine* _instance;

		Renderer* _renderer;
		InputManager* _inputManager;
	};

}
