#include "EngineManager.h"

#include "System.h"

Engine::EngineManager* Engine::EngineManager::_instance = nullptr;

Engine::EngineManager* Engine::EngineManager::GetInstance()
{
	return _instance = _instance ? _instance : DEBUG_NEW EngineManager();
}

void Engine::EngineManager::CleanUp()
{
	delete _instance;
	_instance = nullptr;
}