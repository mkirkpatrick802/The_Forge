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
		
	private:

		void ToggleEditor(const String& args);

	private:
		
		std::unique_ptr<Editor::EditorManager> _editor;

	};

}
