#pragma once
#include <memory>

#include "Engine/Components/Camera.h"

namespace Engine
{
    class CameraManager
    {
    public:
        static std::shared_ptr<CameraManager> GetCameraManager();
        CameraManager();
        
        Camera* GetActiveCamera() const {return _currentCamera; }
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }

        static void CleanUp();
    private:
        
    private:

        static std::shared_ptr<CameraManager> _cameraManager;
        Camera* _currentCamera = nullptr;
        
    };
}
