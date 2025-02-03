#include "System.h"

#include <cassert>
#include <filesystem>
#include <SDL.h>
#include <imgui.h>
#include <SDL_image.h>

#include "EventData.h"
#include "EventSystem.h"

const std::string ERROR_FILENAME = "ErrorFile";

_CrtMemState Engine::System::_memoryCheckpoint = {};
HANDLE Engine::System::_errorFile = INVALID_HANDLE_VALUE;
SDL_Window* Engine::System::_window = nullptr;
glm::vec2 Engine::System::_windowSize = glm::vec2(1280, 720);
Engine::ConsoleStreamBuffer Engine::System::_consoleBuffer;

void Engine::System::Init()
{
	_CrtMemCheckpoint(&_memoryCheckpoint);

	if (std::remove(ERROR_FILENAME.c_str()) != 0)
		LogToErrorFile("Failed to Destroy " + ERROR_FILENAME);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		LogToErrorFile("SDL could not initialize!");
		assert(0);
	}

	std::cout.rdbuf(&_consoleBuffer);
	std::cerr.rdbuf(&_consoleBuffer);
}

void Engine::System::CreateAppWindow(const glm::vec2 windowSize)
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
	_windowSize = glm::vec2(displayMode.w, displayMode.h);

	const auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | ImGuiWindowFlags_NoBringToFrontOnFocus | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_MAXIMIZED);
	_window = SDL_CreateWindow("The Forge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)_windowSize.x, (int)_windowSize.y, window_flags);
	if (_window == nullptr)
	{
		LogToErrorFile("SDL window could not be made!");
		assert(0);
	}

	if (SDL_Surface* logoSurface = IMG_Load("Assets/Sprites/logo.png"))
	{
		SDL_SetWindowIcon(_window, logoSurface);
		SDL_FreeSurface(logoSurface);
	}
}

void Engine::System::CleanUp()
{
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
		// Does this really mean there is a memory leak??
		// Yes it does (maybe)
		DisplayMessageBox("Memory Leak Detected!!", "Check output log for more details.");
	}
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

void Engine::System::DisplayMessageBox(const std::string& caption, const std::string& message)
{
	const std::wstring wCaption(caption.begin(), caption.end());
	const std::wstring wMessage(message.begin(), message.end());

	MessageBox(nullptr, wMessage.c_str(), wCaption.c_str(), MB_OK);
}

void Engine::System::LogToConsole(const char* format, ...)
{
#ifndef _DEBUG
	return;
#endif
	
	constexpr size_t bufferSize = 1024;
	char buffer[bufferSize];

	va_list args;
	va_start(args, format);
	auto temp = std::vsnprintf(buffer, bufferSize, format, args);
	va_end(args);

	ED_LogToConsole log;
	log.message = buffer;
	log.type = LogType::MESSAGE_LOG;
	if (const auto eventSystem = EventSystem::GetInstance())
		eventSystem->TriggerEvent(ED_LogToConsole::GetEventName(), &log);
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
