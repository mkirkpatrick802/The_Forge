#pragma once
#include <memory>

#include "Chat.h"

namespace Engine
{
	class GameObject;
	class Renderer;
	class InputManager;
	class LevelManager;
	class GameEngine
	{
	public:
		static GameEngine& GetInstance();
		
		GameEngine();
		~GameEngine();
		
		void StartGameplayLoop();

	private:
		void SceneStartup();
		void ToggleLoadingScreen(bool enabled);
		
	private:
		std::unique_ptr<Renderer> _renderer;
		std::unique_ptr<InputManager> _inputManager;
		std::unique_ptr<LevelManager> _levelManager;
		std::unique_ptr<Chat> _chat;
		std::unique_ptr<GameObject> _loadingScreen;

	public:
		Renderer& GetRenderer() const { return *_renderer; }
		InputManager& GetInputManager() const { return *_inputManager; }
		LevelManager& GetLevelManager() const { return *_levelManager; }
		
	};

	inline Renderer& GetRenderer()
	{
		return GameEngine::GetInstance().GetRenderer();
	}

	inline InputManager& GetInputManager()
	{
		return GameEngine::GetInstance().GetInputManager();
	}
	
	inline GameEngine& GetGameEngine()
	{
		return GameEngine::GetInstance();
	}

	inline LevelManager& GetLevelManager()
	{
		return GameEngine::GetInstance().GetLevelManager();
	}
}
