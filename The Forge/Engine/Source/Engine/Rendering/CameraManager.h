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
        ~CameraManager();
        
        Camera* GetActiveCamera() const { return _currentCamera; }
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }

        glm::vec2 ConvertWorldToScreen(glm::vec2 worldPos) const;
        glm::vec2 ConvertScreenToWorld(glm::vec2 screenPos) const;

    private:
        void ResetActiveCamera(const void* p);
        
    private:
        Camera* _currentCamera = nullptr;
        
    };

    inline CameraManager& GetCameraManager()
    {
        return CameraManager::GetInstance();
    }
}
