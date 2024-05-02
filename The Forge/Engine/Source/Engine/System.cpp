#include "System.h"

#include <cassert>
#include <SDL.h>
#include <imgui.h>

const String ERROR_FILENAME = "ErrorFile";

_CrtMemState Engine::System::_memoryCheckpoint = {};
HANDLE Engine::System::_errorFile = INVALID_HANDLE_VALUE;
SDL_Window* Engine::System::_window = nullptr;
Vector2D Engine::System::_windowSize = Vector2D(1280, 720);

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
}

void Engine::System::CreateAppWindow(const Vector2D windowSize)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	_windowSize = windowSize;

	const auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | ImGuiWindowFlags_NoBringToFrontOnFocus);
	_window = SDL_CreateWindow("The Forge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)_windowSize.x, (int)_windowSize.y, window_flags);
	if (_window == nullptr)
	{
		LogToErrorFile("SDL window could not be made!");
		assert(0);
	}
}

void Engine::System::CleanUp()
{
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

void Engine::System::LogToErrorFile(const String& message)
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

void Engine::System::DisplayMessageBox(const String& caption, const String& message)
{
	const std::wstring wCaption(caption.begin(), caption.end());
	const std::wstring wMessage(message.begin(), message.end());

	MessageBox(nullptr, wMessage.c_str(), wCaption.c_str(), MB_OK);
}