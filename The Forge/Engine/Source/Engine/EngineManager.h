#pragma once
#include <memory>

#include "Data.h"
#include "json.hpp"


namespace NetCode
{
	class NetObject;
}

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

		static std::shared_ptr<EngineManager> GetInstance();
		EngineManager();

		static void CleanUp();
		~EngineManager();
		
	public:

		static void UpdateConfigFile(const String& file, const String& jsonKeyword, const String& data);
		static nlohmann::json GetConfigData(const String& file, const String& jsonKeyword);
		static bool IsEditorEnabled() {return _editorEnabled; }

		static void UpdateNetObject();
		
		void ToggleEditor(const String& args);
	
	private:
		
		void CreateLobby(const String& args);
		void JoinLobby(const String& args);
		
	private:

		static std::shared_ptr<EngineManager> _instance;
		static bool _editorEnabled;
		std::unique_ptr<Editor::EditorManager> _editor;
		std::shared_ptr<NetCode::NetObject> _netObject;

	};
}
