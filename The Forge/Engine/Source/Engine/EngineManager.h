#pragma once
#include <memory>

#include "Data.h"


namespace Editor
{
	class EditorManager;
}

namespace Engine
{
	class EngineManager
	{

	public:

		EngineManager();
		~EngineManager();
		
	public:

		static void UpdateConfigFile(String jsonKeyword, String data);
		void ToggleEditor(const String& args);

	public:

		
		
	private:
		
		std::unique_ptr<Editor::EditorManager> _editor;

	};

}
