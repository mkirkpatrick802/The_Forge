#pragma once
#include <memory>

#include "Camera.h"

namespace Engine
{
    class CameraManager
    {
    public:
        static std::shared_ptr<CameraManager> GetCameraManager();
        CameraManager() = default;
        
        std::shared_ptr<Camera> GetActiveCamera() {return _currentCamera; }
        void SetActiveCamera(const std::shared_ptr<Camera>& camera) {  _currentCamera = camera; }

        static void CleanUp();
    private:
        
    private:

        static std::shared_ptr<CameraManager> _cameraManager;
        std::shared_ptr<Camera> _currentCamera;
    };
}
