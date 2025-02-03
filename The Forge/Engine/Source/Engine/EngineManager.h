#pragma once
#include <memory>

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
	const std::string CONFIG_PATH = "Config/";
	const std::string DEFAULTS_FILE = "defaults.json";
	class EngineManager
	{

	public:

		static std::shared_ptr<EngineManager> GetInstance();
		EngineManager();

		static void CleanUp();
		~EngineManager();
		
	public:

		static void UpdateConfigFile(const std::string& file, const std::string& jsonKeyword, const std::string& data);
		static nlohmann::json GetConfigData(const std::string& file, const std::string& jsonKeyword);
		static bool IsEditorEnabled() {return _editorEnabled; }

		static void UpdateNetObject();
		
		void ToggleEditor(const std::string& args);
	
	private:
		
		void CreateLobby(const std::string& args);
		void JoinLobby(const std::string& args);
		
	private:

		static std::shared_ptr<EngineManager> _instance;
		static bool _editorEnabled;
		std::unique_ptr<Editor::EditorManager> _editor;
		std::shared_ptr<NetCode::NetObject> _netObject;

	};
}
