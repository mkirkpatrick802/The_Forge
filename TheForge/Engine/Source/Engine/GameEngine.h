#pragma once
#include <memory>

#include "Chat.h"
#include "ServerConsole.h"

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
		// Registered as the "Editor Enabled" event handler, and only that.
		//
		// Never opens the main menu, whatever the config says: this fires when the
		// editor is toggled, and the editor's Play button toggles it off on its way into
		// StartCurrentLevel. Loading the menu here would mean every Play loaded the menu
		// and then immediately threw it away.
		void SceneStartup(const void* p = nullptr);

		// The level this process opens on. The menu, for a windowed client with one
		// configured; the default level for the editor, a dedicated server and any build
		// with no menu.
		void LoadStartupLevel(bool allowMainMenu);
		
	private:
		std::unique_ptr<Renderer> _renderer;
		std::unique_ptr<InputManager> _inputManager;
		std::unique_ptr<LevelManager> _levelManager;
		std::unique_ptr<Chat> _chat;
		std::unique_ptr<ServerConsole> _console;

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
