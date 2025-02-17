#pragma once
#include <memory>

#include "Engine/Components/Camera.h"

namespace Engine
{
    class CameraManager
    {
    public:
        static CameraManager& GetInstance();
        CameraManager();
        
        Camera* GetActiveCamera() const {return _currentCamera; }
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }

        glm::vec2 ConvertWorldToScreen(glm::vec2 worldPos);
        glm::vec2 ConvertScreenToWorld(glm::vec2 screenPos);
        
    private:
        Camera* _currentCamera = nullptr;
        
    };

    inline CameraManager& GetCameraManager()
    {
        return CameraManager::GetInstance();
    }
}
