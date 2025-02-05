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

#define DEBUG_PRINT(Format, ...) Engine::System::GetInstance().LogToConsole(Format, ##__VA_ARGS__);

namespace Engine
{
	enum class LogType : uint8_t {MESSAGE_LOG = 1, WARNING_LOG, ERROR_LOG}; 
	
	typedef SDL_Window Window;

	class System
	{
	public:

		static System& GetInstance();
		System();
		~System();
		
		Window* CreateAppWindow();

		void LogToErrorFile(const std::string& message);
		void DisplayMessageBox(const std::string& caption, const std::string& message) const;
		void LogToConsole(const char* format, ...) const;

		Window* GetWindow() const { return _window; }
		glm::vec2 GetWindowSize() const;

		// File Helpers
		static void EnsureDirectoryExists(const std::string& path);
		

	private:

		_CrtMemState _memoryCheckpoint;
		HANDLE _errorFile;
		
		Window* _window;

		ConsoleStreamBuffer _consoleBuffer;
	};

	inline Window* CreateAppWindow()
	{
		return System::GetInstance().CreateAppWindow();
	}
	
	inline Window* GetAppWindow()
	{
		return System::GetInstance().GetWindow();
	}

	inline glm::vec2 GetAppWindowSize()
	{
		return System::GetInstance().GetWindowSize();
	}
	
}