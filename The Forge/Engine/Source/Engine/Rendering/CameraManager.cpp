#include "CameraManager.h"

std::shared_ptr<Engine::CameraManager> Engine::CameraManager::_cameraManager;
std::shared_ptr<Engine::CameraManager> Engine::CameraManager::GetCameraManager()
{
    if (_cameraManager == nullptr) _cameraManager = std::make_shared<CameraManager>();

    return _cameraManager;
}

void Engine::CameraManager::CleanUp()
{
    _cameraManager.reset();
}
