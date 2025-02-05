#include "CameraManager.h"

#include "Editor/EditorCamera.h"
#include "Engine/EngineManager.h"

Engine::CameraManager& Engine::CameraManager::GetCameraManager()
{
    static auto instance = std::make_unique<CameraManager>();
    return *instance;
}

Engine::CameraManager::CameraManager()
{
    
}