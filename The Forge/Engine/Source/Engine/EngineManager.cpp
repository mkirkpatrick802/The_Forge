#include "EngineManager.h"

#include <fstream>

#include "CommandRegistry.h"
#include "CommandUtils.h"
#include "Components/Component.h"
#include "EventSystem.h"
#include "GameEngine.h"
#include "InputManager.h"
#include "System.h"
#include "Editor/EditorManager.h"

Engine::EngineManager& Engine::EngineManager::GetInstance()
{
	static auto instance = std::make_unique<EngineManager>();
	return *instance;
}

Engine::EngineManager::EngineManager()
{
	CommandRegistry::RegisterCommand("/editor", [this](const std::string& args){ ToggleEditor(args); });
	
	//Find config file & if it does not exist make one
	System::EnsureDirectoryExists(CONFIG_PATH);
	
	const std::string filepath = CONFIG_PATH + DEFAULTS_FILE;
	if (!std::filesystem::exists(filepath))
	{
		std::ofstream defaults(filepath);
		const nlohmann::json data;
		defaults << data.dump(4);
		defaults.close();
	}
}

Engine::EngineManager::~EngineManager()
{
	_netObject.reset();
	
	_editor.reset();
	CommandRegistry::UnregisterCommand("/editor");
	CommandRegistry::UnregisterCommand("/create");
	CommandRegistry::UnregisterCommand("/join");
	EventSystem::DestroyInstance();
}

void Engine::EngineManager::ToggleEditor(const std::string& args)
{
	try
	{
		if (CommandUtils::ParseBoolean(args))
		{
			_editorEnabled = true;
			if (_editor != nullptr) return;

			EventSystem::GetInstance()->TriggerEvent("Editor Enabled");
			_editor = std::make_unique<Editor::EditorManager>();
		}
		else
		{
			_editorEnabled = false;
			if (_editor == nullptr) return;
			_editor.reset();
		}
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
	}
}

void Engine::EngineManager::CollectDebugInputs()
{
	if (GetInputManager().GetKey(SDL_SCANCODE_LSHIFT) && GetInputManager().GetKeyDown(SDL_SCANCODE_F1))
		ToggleEditor("1");
}

void Engine::EngineManager::UpdateConfigFile(const std::string& file, const std::string& jsonKeyword, const std::string& data) const
{
	const std::string path = CONFIG_PATH + file;
	
	// Read in config
	std::ifstream input(path);
	if (!input.is_open()) return;
	
	nlohmann::json currentData;
	input >> currentData;
	currentData[jsonKeyword] = data;
	input.close();

	// Output
	std::ofstream output(path);
	if (!output.is_open()) return;
	output << currentData.dump(4);
	output.close();
}

nlohmann::json Engine::EngineManager::GetConfigData(const std::string& file, const std::string& jsonKeyword)
{
	const std::string path = CONFIG_PATH + file;
	
	std::ifstream config(path);
	if (!config.is_open()) return {};
	
	nlohmann::json currentData;
	config >> currentData;
	
	return currentData[jsonKeyword];
}