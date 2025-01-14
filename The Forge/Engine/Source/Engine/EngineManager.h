#pragma once
#include <memory>

#include "Data.h"
#include "json.hpp"


namespace Editor
{
	class EditorManager;
}

namespace Engine
{
	const String CONFIG_PATH = "Config/";
	const String DEFAULTS_FILE = "defaults.json";
	class EngineManager
	{

	public:

		EngineManager();
		~EngineManager();
		
	public:

		static void UpdateConfigFile(const String& file, const String& jsonKeyword, const String& data);
		static nlohmann::json GetConfigData(const String& file, const String& jsonKeyword);
		
		void ToggleEditor(const String& args);

	public:

		
		
	private:
		
		std::unique_ptr<Editor::EditorManager> _editor;

	};

}
