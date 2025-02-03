#pragma once

#define CRTDBG_MAP_ALLOC
#include "Windows.h"

#include <iostream>
#include <SDL_render.h>
#include <glm/vec2.hpp>

#include "ConsoleStreamBuffer.h"

#ifdef _DEBUG
#define DEBUG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif

namespace Engine
{
#define DEBUG_PRINT(Format, ...) System::LogToConsole(Format, ##__VA_ARGS__);
	enum class LogType : uint8_t {MESSAGE_LOG = 1, WARNING_LOG, ERROR_LOG}; 
	
	typedef SDL_Window Window;

	class System
	{
	public:

		static void Init();
		static void CreateAppWindow(glm::vec2 windowSize = glm::vec2(1280, 720));
		static void CleanUp();

		static void LogToErrorFile(const std::string& message);
		static void DisplayMessageBox(const std::string& caption, const std::string& message);
		static void LogToConsole(const char* format, ...);

		static Window* GetWindow() { return _window; }
		static glm::vec2 GetWindowSize() { return _windowSize; }

		// File Helpers
		static void EnsureDirectoryExists(const std::string& path);
		

	private:

		static _CrtMemState _memoryCheckpoint;
		static HANDLE _errorFile;

		static glm::vec2 _windowSize;
		static Window* _window;

		static ConsoleStreamBuffer _consoleBuffer;
	};
}