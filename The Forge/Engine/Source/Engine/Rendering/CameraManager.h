#pragma once
#include <memory>

#include "Engine/Components/Camera.h"

namespace Engine
{
    class CameraManager
    {
    public:
        
        static CameraManager& GetCameraManager();
        CameraManager();
        
        Camera* GetActiveCamera() const {return _currentCamera; }
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }
        
    private:
        
        Camera* _currentCamera = nullptr;
        
    };
}
