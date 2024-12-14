#pragma once

namespace Engine
{
	class CommandTerminal;
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
		void OpenCommandTerminal();
		void CloseCommandTerminal();
		
	private:

		static GameEngine* _instance;

		CommandTerminal* _terminal = nullptr;
		Renderer* _renderer;
		InputManager* _inputManager;
		LevelManager* _levelManager;
	};
}
