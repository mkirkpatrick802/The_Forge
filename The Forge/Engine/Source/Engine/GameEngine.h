#pragma once

namespace Engine
{
	class Renderer;
	class InputManager;
	class LevelManager;
	
	class GameEngine
	{
	public:

		static GameEngine* GetInstance();
		void StartGamePlayLoop();
		void CleanUp();
	
	private:
		
		GameEngine();
		
	private:

		static GameEngine* _instance;
		
		Renderer* _renderer;
		InputManager* _inputManager;
		LevelManager* _levelManager;
	};
}
