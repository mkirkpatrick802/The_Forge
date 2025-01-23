#pragma once
#include <memory>

#include "Chat.h"

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
		std::shared_ptr<InputManager> _inputManager;
		LevelManager* _levelManager;
		
		std::unique_ptr<Chat> _chat;
	};
}
