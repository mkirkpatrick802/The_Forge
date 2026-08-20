#include "GameEngine.h"

#include "GameSession.h"

#include <fstream>
#include <SDL_timer.h>

#include "EngineManager.h"
#include "EventSystem.h"
#include "GameObject.h"
#include "InputManager.h"
#include "JsonKeywords.h"
#include "LaunchOptions.h"
#include "Level.h"
#include "LevelManager.h"
#include "NetworkManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "../../../Netcode/Source/GamerServices.h"
#include "../../../Netcode/Source/Transport/UdpTransport.h"
#include "Collisions/CollisionManager.h"
#include "Components/ComponentManager.h"
#include "Components/SpriteRenderer.h"
#include "Editor/EditorManager.h"
#include <SDL_scancode.h>

#include "UI/PauseScreen.h"
#include "UI/SettingsScreen.h"
#include "UI/UIDebugScreen.h"
#include "UI/UIRoot.h"

Engine::GameEngine& Engine::GameEngine::GetInstance()
{
	static auto instance = std::make_unique<GameEngine>();
	return *instance;
}

Engine::GameEngine::GameEngine()
{
	_renderer = std::make_unique<Renderer>();
	_inputManager = std::make_unique<InputManager>();

	// Chat is an ImGui window driven by local key presses -- presentation only, and
	// there is no ImGui context when headless.
	if (!GetLaunchOptions().headless)
		_chat = std::make_unique<Chat>();
	else
		_console = std::make_unique<ServerConsole>();

	// Registered even in the editor: the test card is the quickest way to confirm the
	// UI layer still draws after a change, and the command terminal is in the editor.
	if (!GetLaunchOptions().headless)
	{
		UIDebugScreen::RegisterCommand();
		GameSession::RegisterCommands();
	}

	EventSystem::GetInstance()->RegisterEvent("Editor Enabled", this, &GameEngine::SceneStartup);
}

Engine::GameEngine::~GameEngine()
{
	_levelManager->CleanUp();

	EventSystem::GetInstance()->DeregisterEvent("Editor Enabled", this);
}

void Engine::GameEngine::SceneStartup(const void* p)
{
	LoadStartupLevel(false);
}

void Engine::GameEngine::LoadStartupLevel(const bool allowMainMenu)
{
	std::string filename;

	// A menu is for a player looking at a window. A dedicated server hosts rather than
	// joins, and the editor is an authoring tool -- both want the level they are there
	// to work on, not a title screen in front of it.
	const bool wantsMenu = allowMainMenu
		&& !GetLaunchOptions().headless
		&& !GetLaunchOptions().IsDedicatedServer()
		&& !GetEngineManager().IsEditorEnabled();

	if (wantsMenu)
	{
		// Absent means this build has no menu, which is what every build was before one
		// existed -- so it boots straight into its default level rather than failing.
		filename = GameSession::GetMainMenuLevel();
	}

	if (filename.empty())
	{
		if (const auto defaultData = GetEngineManager().GetConfigData(DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL);
			defaultData.is_string())
			filename = defaultData.get<std::string>();
	}

	if (filename.empty())
	{
		System::GetInstance().DisplayMessageBox("ERROR", "Could not load default level!!");
		return;
	}

	const std::string filepath = LEVEL_PATH + filename + ".json";

	if (_levelManager)
		LevelManager::LoadLevel(filepath);
	else
		_levelManager = std::make_unique<LevelManager>(filepath);
}

void Engine::GameEngine::StartGameplayLoop()
{
	const bool headless = GetLaunchOptions().headless;

	// Before the first frame, so a resolution or fullscreen choice from a previous
	// session is in effect rather than snapping into place once the player opens
	// settings.
	if (!headless)
		SettingsScreen::ApplyStoredSettings();

	// The one place the main menu is allowed to be the startup level -- see the note on
	// SceneStartup, which does the same work minus that.
	LoadStartupLevel(true);

	if (headless)
		DEBUG_LOG("Scene loaded. Dedicated server running headless.")


	float frameStart = static_cast<float>(Time::GetTicks());
	if (!headless)
		_inputManager->ClearInputBuffers();

	// Headless has no window to pump events for, so the loop runs until something
	// asks the application to close rather than until the input manager says stop.
	while (headless ? !APPLICATION_CLOSING : _inputManager->StartProcessInputs())
	{
		if (const float currentTicks = static_cast<float>(Time::GetTicks()); currentTicks - frameStart >= 16)
		{
			const float deltaTime = (currentTicks - frameStart) / 1000.f;
			Time::SetDeltaTime(deltaTime);
			frameStart = currentTicks;
			
			if (!GetEngineManager().IsEditorEnabled())
			{
				GetCollisionManager().Update();

				// One call for every role. Whichever transport this process picked
				// is pumped from inside, so nothing here needs to know whether Steam,
				// UDP or nothing at all is underneath.
				NetCode::GetNetworkManager().Update();

				if (_chat)
					_chat->Update(deltaTime);

				// Before the components tick, so a button press is acted on in the same
				// frame it was made rather than a frame later -- and so a screen that
				// blocks input has already said so by the time gameplay reads the mouse.
				if (!headless)
				{
					// Esc opens the pause menu, and the pause menu's own Resume closes
					// it. Only while a level is actually being played: on the main menu
					// there is nothing to pause, and while a loading screen is up the
					// session is still being established.
					if (GetInputManager().GetKeyDown(SDL_SCANCODE_ESCAPE)
						&& UIRoot::IsEmpty()
						&& !GameSession::IsInMainMenu())
					{
						UIRoot::Push(std::make_unique<PauseScreen>());
					}

					UIRoot::Update(deltaTime);

					// After the UI, so a Play pressed this frame has already put the
					// loading screen up and started the session by the time this looks
					// at it.
					GameSession::Update(deltaTime);
				}

				if (_console)
					_console->Update();

				GetComponentManager().UpdateComponents(deltaTime);

				// Match rules tick after the world does, so a phase change reacts to the
				// state this frame produced rather than last frame's. A no-op on a client:
				// a level that arrived over the wire deliberately has no game mode.
				if (const Level* level = LevelManager::GetCurrentLevel())
					level->UpdateGameMode(deltaTime);
			}
			else
			{
				GetEditorManager().Update();
			}

#if DEBUG
			GetEngineManager().CollectDebugInputs();			
#endif
			
			if (!headless)
			{
				_renderer->Render();
				_inputManager->EndProcessInputs();
			}
		}
		else if (headless)
		{
			// Nothing to draw and no events to pump, so yield instead of burning a
			// core spinning on the frame-time check.
			SDL_Delay(1);
		}
	}
}
