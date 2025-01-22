#include "EngineManager.h"

#include <fstream>

#include "CommandRegistry.h"
#include "CommandUtils.h"
#include "Component.h"
#include "EventSystem.h"
#include "JsonKeywords.h"
#include "System.h"
#include "Editor/EditorManager.h"

Engine::EngineManager::EngineManager()
{
	CommandRegistry::RegisterCommand("/editor", [this](const String& args){ ToggleEditor(args); });

	//Find config file & if it does not exist make one
	System::EnsureDirectoryExists(CONFIG_PATH);
	
	const String filepath = CONFIG_PATH + DEFAULTS_FILE;
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
	_editor.reset();
	CommandRegistry::UnregisterCommand("/editor");
	EventSystem::DestroyInstance();
}

void Engine::EngineManager::ToggleEditor(const String& args)
{
	try
	{
		if (CommandUtils::ParseBoolean(args))
		{
			if (_editor != nullptr) return;
			_editor = std::make_unique<Editor::EditorManager>();
		}
		else
		{
			if (_editor == nullptr) return;
			_editor.reset();
		}
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
	}
}

void Engine::EngineManager::UpdateConfigFile(const String& file, const String& jsonKeyword, const String& data)
{
	const String path = CONFIG_PATH + file;
	
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

nlohmann::json Engine::EngineManager::GetConfigData(const String& file, const String& jsonKeyword)
{
	const String path = CONFIG_PATH + file;
	
	std::ifstream config(path);
	if (!config.is_open()) return {};
	
	nlohmann::json currentData;
	config >> currentData;
	
	return currentData[jsonKeyword];
}
