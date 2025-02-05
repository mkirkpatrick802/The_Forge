#include "CameraManager.h"

#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"
#include "Engine/EventData.h"
#include "Engine/EventSystem.h"

std::shared_ptr<Engine::CameraManager> Engine::CameraManager::_cameraManager;

std::shared_ptr<Engine::CameraManager> Engine::CameraManager::GetCameraManager()
{
    if (_cameraManager == nullptr) _cameraManager = std::make_shared<CameraManager>();

    return _cameraManager;
}

Engine::CameraManager::CameraManager()
{
    
}

void Engine::CameraManager::CleanUp()
{
    _cameraManager.reset();
}
