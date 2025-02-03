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

void Engine::CameraManager::CreateCamera(const void* data)
{
    // const auto eventData = static_cast<const ED_CreateComponent*>(data);
    // if (!eventData || eventData->componentID != CAMERA) return;
    //
    // // These are very necessary do not get rid of
    // Engine::GameObject* go = eventData->gameObject;
    // auto camera = _cameraPool.New(static_cast<Engine::GameObject*>(go));
    //
    // if (eventData->data != nullptr)
    //     camera->LoadData(eventData->data);
    //
    // eventData->gameObject->AddComponent(camera);
    //
    // if (_currentCamera == nullptr)
    //     _currentCamera = camera;
}

void Engine::CameraManager::DeleteCamera(const void* data)
{
    // const auto eventData = static_cast<const ED_DestroyComponent*>(data);
    // if (!eventData) return;
    //
    // const auto camera = dynamic_cast<Camera*>(eventData->component);
    // if (camera == nullptr) return;
	   //
    // _cameraPool.Delete(camera);
}

void Engine::CameraManager::CleanUp()
{
    _cameraManager.reset();
}
