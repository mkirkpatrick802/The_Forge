#include "EngineManager.h"

#include "CommandRegistry.h"
#include "CommandUtils.h"
#include "System.h"
#include "Editor/EditorManager.h"

Engine::EngineManager::EngineManager()
{
	CommandRegistry::RegisterCommand("/editor", [this](const String& args){ ToggleEditor(args); });
}

Engine::EngineManager::~EngineManager()
{
	CommandRegistry::UnregisterCommand("/editor");
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

void Engine::EngineManager::UpdateConfigFile(String jsonKeyword, String data)
{
	
}