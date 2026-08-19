#include "System.h"

#include <cassert>
#include <filesystem>
#include <process.h>
#include <SDL.h>
#include <imgui.h>
#include <SDL_image.h>

#include "EventData.h"
#include "EventSystem.h"
#include "../../../Netcode/Source/GamerServices.h"
#include "LaunchOptions.h"

const std::string ERROR_FILENAME = "ErrorFile";

Engine::System& Engine::System::GetInstance()
{
	static auto instance = std::make_unique<System>();
	return *instance;
}

Engine::System::System(): _errorFile(nullptr), _window(nullptr)
{
	_CrtMemCheckpoint(&_memoryCheckpoint);

	if (REQUIRE_GAMER_SERVICES)
		NetCode::GamerServices::Init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LogToErrorFile("SDL could not initialize!");
		assert(0);
	}

	if (std::remove(ERROR_FILENAME.c_str()) != 0)
		LogToErrorFile("Failed to Destroy " + ERROR_FILENAME);

	if (GetLaunchOptions().headless)
	{
		// This is a WindowedApp, so it has no console of its own. Make one, otherwise
		// a dedicated server's output has nowhere to go. std::cout is deliberately
		// left alone here so it reaches that console instead of the in-engine one.
		if (AllocConsole())
		{
			FILE* stream = nullptr;
			freopen_s(&stream, "CONOUT$", "w", stdout);
			freopen_s(&stream, "CONOUT$", "w", stderr);
		}
	}
	else
	{
		std::cout.rdbuf(&_consoleBuffer);
		std::cerr.rdbuf(&_consoleBuffer);
	}
}

Engine::System::~System()
{
	if (REQUIRE_GAMER_SERVICES)
		NetCode::GamerServices::CleanUp();

	_consoleBuffer.CleanUp();
	
	SDL_DestroyWindow(_window);
	SDL_Quit();

	if (_errorFile != INVALID_HANDLE_VALUE)
		CloseHandle(_errorFile);

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	_CrtMemDumpAllObjectsSince(&_memoryCheckpoint);

	_CrtMemState newCheckpoint;
	_CrtMemCheckpoint(&newCheckpoint);

	_CrtMemState difference;
	if (_CrtMemDifference(&difference, &_memoryCheckpoint, &newCheckpoint))
	{
		DisplayMessageBox("Memory Leak Detected!!", "Check output log for more details.");
	}
}

SDL_Window* Engine::System::CreateAppWindow(const glm::vec2& size)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	// A positive size requests a specific window size (used by the Launcher).
	// Default-constructed size keeps the original behaviour: fill the display
	// and open maximized.
	const bool useRequestedSize = size.x > 0.0f && size.y > 0.0f;

	auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | ImGuiWindowFlags_NoBringToFrontOnFocus | SDL_WINDOW_MOUSE_CAPTURE);
	if (!useRequestedSize)
		window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_MAXIMIZED);

	const int windowWidth = useRequestedSize ? (int)size.x : displayMode.w;
	const int windowHeight = useRequestedSize ? (int)size.y : displayMode.h;

	_window = SDL_CreateWindow("The Forge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, window_flags);
	if (_window == nullptr)
	{
		LogToErrorFile("SDL window could not be made!");
		assert(0);
	}

	if (SDL_Surface* logoSurface = IMG_Load("Assets/Engine Assets/Sprites/logo.png"))
	{
		SDL_SetWindowIcon(_window, logoSurface);
		SDL_FreeSurface(logoSurface);
	}

	return _window;
}

glm::vec2 Engine::System::GetWindowSize() const
{
	int width, height;
	SDL_GetWindowSize(_window, &width, &height);
	return glm::vec2(width, height);
}

void Engine::System::LogToErrorFile(const std::string& message)
{
	if (_errorFile == INVALID_HANDLE_VALUE)
	{
		_errorFile = CreateFileW(L"ErrorFile", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (!SUCCEEDED(_errorFile))
			DisplayMessageBox("File Not Written", "Couldn't write error to error file.");
	}

	DWORD bytesWritten;
	WriteFile(_errorFile, message.c_str(), strlen(message.c_str()), &bytesWritten, nullptr);
}

void Engine::System::DisplayMessageBox(const std::string& caption, const std::string& message) const
{
	const std::wstring wCaption(caption.begin(), caption.end());
	const std::wstring wMessage(message.begin(), message.end());

	MessageBox(nullptr, wMessage.c_str(), wCaption.c_str(), MB_OK);
}

void Engine::System::LogToConsole(const char* format, ...) const
{
	constexpr size_t bufferSize = 1024;
	char buffer[bufferSize];

	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, bufferSize, format, args);
	va_end(args);

	// A headless build has no in-engine console to receive the event, so logs go
	// straight to stdout. Deliberately not gated on _DEBUG -- a dedicated server
	// needs its log in Release too.
	if (GetLaunchOptions().headless)
	{
		std::fputs(buffer, stdout);
		std::fputc('\n', stdout);
		std::fflush(stdout);

		// Also mirrored to a file: a dedicated server's console is often not where
		// anyone is looking, and this survives a crash.
		//
		// The server keeps a fixed name -- there is one per machine, and the run
		// scripts and docs point at it. Clients are suffixed with their process id,
		// because testing this thing means running several from the same folder and a
		// shared name interleaved them all into one unreadable file.
		static const std::string logName = GetLaunchOptions().IsDedicatedServer()
			? std::string("server.log")
			: "client-" + std::to_string(_getpid()) + ".log";

		if (FILE* logFile = nullptr; fopen_s(&logFile, logName.c_str(), "a") == 0 && logFile)
		{
			std::fputs(buffer, logFile);
			std::fputc('\n', logFile);
			std::fclose(logFile);
		}
		return;
	}

#ifndef _DEBUG
	return;
#endif

	ED_LogToConsole log;
	log.message = buffer;
	log.type = LogType::MESSAGE_LOG;
	if (const auto eventSystem = EventSystem::GetInstance())
	{
		eventSystem->TriggerEvent(ED_LogToConsole::GetEventName(), &log);
	}
}

void Engine::System::EnsureDirectoryExists(const std::string& path)
{
	std::filesystem::path dir(path);
	for (auto it = dir.begin(); it != dir.end(); ++it)
	{
		std::filesystem::path currentPath;
		for (auto sub_it = dir.begin(); sub_it != std::next(it); ++sub_it)
		{
			currentPath /= *sub_it;
		}

		if (!exists(currentPath))
		{
			create_directory(currentPath);
			std::cout << "Created directory: " << currentPath << '\n';
		}
	}
}
