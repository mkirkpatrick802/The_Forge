#include "System.h"

#include <iostream>

namespace Engine
{
	void System::Init()
	{
		if (AllocConsole()) DisplayMessageBox("Console Failed to Open");
		freopen("CONOUT$", "w", stdout);
	}

	void System::PreAppStartUp()
	{
		_CrtMemCheckpoint(&_memoryCheckpoint);
	}

	void System::PostAppStartUp()
	{
		_CrtMemDumpAllObjectsSince(&_memoryCheckpoint);

		_CrtMemState newCheckpoint;
		_CrtMemState difference;
		_CrtMemCheckpoint(&newCheckpoint);

		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

		//_CrtSetAllocHook(AllocationHook);

		if (_CrtMemDifference(&difference, &_memoryCheckpoint, &newCheckpoint))
			printf("Memory Leak Detected \n");

		//_CrtSetAllocHook(nullptr);
	}

	void System::Shutdown()
	{
		if (_errorFile != INVALID_HANDLE_VALUE)
			CloseHandle(_errorFile);
	}

	void System::DisplayMessageBox(const String& message)
	{
		String caption = "Message Caption";
		std::wstring wCaption(caption.begin(), caption.end());
		std::wstring wMessage(message.begin(), message.end());

		MessageBox(nullptr, wMessage.c_str(), wMessage.c_str(), MB_OK);
	}

	int System::AllocationHook(int allocType, void* userData, size_t size, int blockType, long requestNumber,
							   const unsigned char* filename, int lineNumber)
	{
		if (allocType != _HOOK_ALLOC) return TRUE;

		// Memory allocation
		char message[200];
		snprintf(message, sizeof(message), "Memory allocated at %s:%d\n", filename, lineNumber);
		std::cout << message;

		return TRUE; // Continue allocation/deallocation
	}
}
