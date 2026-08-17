#pragma once

#define CRTDBG_MAP_ALLOC
#include "Windows.h"

#include <iostream>
#include <glm/vec2.hpp>

#include "ConsoleStreamBuffer.h"

#ifdef _DEBUG
#define DEBUG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif

#define DEBUG_LOG(Format, ...) Engine::System::GetInstance().LogToConsole(Format, ##__VA_ARGS__);

inline bool APPLICATION_CLOSING = false;

struct SDL_Window;
namespace Engine
{
	enum class LogType : uint8_t {MESSAGE_LOG = 1, WARNING_LOG, ERROR_LOG};

	// Steam is initialized by the System constructor and hard-exits the process if
	// no Steam client is running. Tools that use no netcode (the Launcher) opt out
	// by setting this to false before the first System::GetInstance() call. Games
	// leave it true. Do not set this false in anything that touches netcode --
	// GamerServices stays uninitialized, so GetGamerService() would deref null.
	inline bool REQUIRE_GAMER_SERVICES = true;

	class System
	{
	public:

		static System& GetInstance();
		System();
		~System();
		
		// A positive size requests that window size; the default fills the display.
		SDL_Window* CreateAppWindow(const glm::vec2& size = glm::vec2(0.0f));

		void LogToErrorFile(const std::string& message);
		void DisplayMessageBox(const std::string& caption, const std::string& message) const;
		void LogToConsole(const char* format, ...) const;

		SDL_Window* GetWindow() const { return _window; }
		glm::vec2 GetWindowSize() const;

		// File Helpers
		static void EnsureDirectoryExists(const std::string& path);
		

	private:

		_CrtMemState _memoryCheckpoint;
		HANDLE _errorFile;
		
		SDL_Window* _window;

		ConsoleStreamBuffer _consoleBuffer;
	};

	inline SDL_Window* CreateAppWindow(const glm::vec2& size = glm::vec2(0.0f))
	{
		return System::GetInstance().CreateAppWindow(size);
	}
	
	inline SDL_Window* GetAppWindow()
	{
		return System::GetInstance().GetWindow();
	}

	inline glm::vec2 GetAppWindowSize()
	{
		return System::GetInstance().GetWindowSize();
	}
	
}