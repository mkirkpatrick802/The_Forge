#pragma once
#include <memory>

#include "Engine/Components/Camera.h"
#include "Engine/Components/ComponentPool.h"

namespace Engine
{
    class CameraManager
    {
    public:
        static std::shared_ptr<CameraManager> GetCameraManager();
        CameraManager();

        void CreateCamera(const void* data);
        void DeleteCamera(const void* data);
        
        Camera* GetActiveCamera() const {return _currentCamera; }
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }

        static void CleanUp();
    private:
        
    private:

        static std::shared_ptr<CameraManager> _cameraManager;
        Camera* _currentCamera = nullptr;
        ComponentPool<Camera> _cameraPool;
    };
}
