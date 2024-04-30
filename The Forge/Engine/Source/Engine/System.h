#pragma once

#include <iostream>

#include "Windows.h"
#include "Data.h"

namespace Engine
{
	class System
	{
	public:

		void Init();
		void PreAppStartUp();
		void PostAppStartUp();
		void Shutdown();

		static void DisplayMessageBox(const String& message);
		static int AllocationHook(int allocType, void* userData, size_t size, int blockType, long requestNumber, const unsigned char* filename, int lineNumber);

	private:

		_CrtMemState _memoryCheckpoint = {};
		HANDLE _errorFile = INVALID_HANDLE_VALUE;

	};
}