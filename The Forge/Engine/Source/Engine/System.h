#pragma once

#define CRTDBG_MAP_ALLOC
#include "Windows.h"

#include <iostream>
#include <SDL_render.h>

#include "Data.h"

#ifdef _DEBUG
#define DEBUG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif
namespace Engine
{
	typedef SDL_Window Window;

	class System
	{
	public:

		static void Init();
		static void CreateAppWindow(Vector2D windowSize = Vector2D(1280, 720));
		static void CleanUp();

		static void LogToErrorFile(const String& message);
		static void DisplayMessageBox(const String& caption, const String& message);

		static Window* GetWindow() { return _window; }
		static Vector2D GetWindowSize() { return _windowSize; }

		// File Helpers
		static void EnsureDirectoryExists(const String& path);
		

	private:

		static _CrtMemState _memoryCheckpoint;
		static HANDLE _errorFile;

		static Vector2D _windowSize;
		static Window* _window;
	};
}