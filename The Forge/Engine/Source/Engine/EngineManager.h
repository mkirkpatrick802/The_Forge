#pragma once

namespace Engine
{
	enum class Mode
	{
		Null = 0,
		Play,
		Edit
	};

	class EngineManager
	{
	public:

		static EngineManager* GetInstance();
		static void CleanUp();

	private:

		EngineManager() = default;

	public:

		Mode mode;

	private:

		static EngineManager* _instance;

	};

}