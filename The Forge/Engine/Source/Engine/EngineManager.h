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

		static EngineManager& GetInstance();
		EngineManager();
		~EngineManager();
		
	public:

		void UpdateConfigFile(const std::string& file, const std::string& jsonKeyword, const std::string& data);
		nlohmann::json GetConfigData(const std::string& file, const std::string& jsonKeyword);
		bool IsEditorEnabled() const {return _editorEnabled; }
		
		void ToggleEditor(const std::string& args);
		
	private:
		
		bool _editorEnabled;
		std::unique_ptr<Editor::EditorManager> _editor;
		std::shared_ptr<NetCode::NetObject> _netObject;
	};

	inline EngineManager& GetEngineManager()
	{
		return EngineManager::GetInstance();
	}
}
