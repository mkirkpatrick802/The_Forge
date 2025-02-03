#include "EngineManager.h"

#include <fstream>

#include "CommandRegistry.h"
#include "CommandUtils.h"
#include "Components/Component.h"
#include "EventSystem.h"
#include "System.h"
#include "../../../Netcode/Source/Utilites/NetworkWrapper.h"
#include "Editor/EditorManager.h"
#include "../../../Netcode/Source/Utilites/NetObject.h"

bool Engine::EngineManager::_editorEnabled = false;
std::shared_ptr<Engine::EngineManager> Engine::EngineManager::_instance = nullptr;

std::shared_ptr<Engine::EngineManager> Engine::EngineManager::GetInstance()
{
	_instance = _instance == nullptr ? std::make_shared<EngineManager>() : _instance;
	return _instance;
}

Engine::EngineManager::EngineManager()
{
	CommandRegistry::RegisterCommand("/editor", [this](const std::string& args){ ToggleEditor(args); });
	CommandRegistry::RegisterCommand("/create", [this](const std::string& args) { CreateLobby(args); });
	CommandRegistry::RegisterCommand("/join", [this](const std::string& args) { JoinLobby(args); });

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

	if ( !NetCode::InitNetcodeBackend() )
	{
		System::DisplayMessageBox("Error","Failed to initialize Netcode backend!!");
	}
}

void Engine::EngineManager::CleanUp()
{
	_instance.reset();
}

Engine::EngineManager::~EngineManager()
{
	_netObject.reset();
	NetCode::ShutdownNetcodeBackend();
	
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
			if (_editor != nullptr) return;

			EventSystem::GetInstance()->TriggerEvent("Editor Enabled");
			_editor = std::make_unique<Editor::EditorManager>();
			_editorEnabled = true;
		}
		else
		{
			if (_editor == nullptr) return;
			_editor.reset();
			_editorEnabled = false;
		}
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
	}
}

void Engine::EngineManager::CreateLobby(const std::string& args)
{
	if (_netObject) { std::cout << "Already in-game" << '\n'; return; };

	_netObject = NetCode::CreateLobby(args);
}

void Engine::EngineManager::JoinLobby(const std::string& args)
{
	if (_netObject) { std::cout << "Already in-game" << '\n'; return; }
	
	_netObject = NetCode::JoinLobby(args);
}

void Engine::EngineManager::UpdateConfigFile(const std::string& file, const std::string& jsonKeyword, const std::string& data)
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

void Engine::EngineManager::UpdateNetObject()
{
	if (_instance == nullptr) GetInstance();
	if (_instance->_netObject == nullptr) return;
	
	_instance->_netObject->Update();
}