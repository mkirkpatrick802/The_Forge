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
        
        // Falls back to the first live Camera component when nothing has been set.
        //
        // The active camera is cleared whenever the editor is enabled, and only
        // LevelManager::StartCurrentLevel ever set it again -- so leaving the editor any
        // other way (the /editor command) left the game with no camera at all, and
        // CameraHelper then handed the renderer a zero matrix, which collapses every
        // vertex to a point. A blank screen for want of one assignment.
        Camera* GetActiveCamera() const;
        void SetActiveCamera(Camera* camera) {  _currentCamera = camera; }

        // Drops the cached pointer if it is this camera. Called by Camera as it goes
        // back to its pool: the cache is a raw pointer into a pool slot that is about
        // to be destroyed, and GetActiveCamera hands out whatever is cached without
        // checking, so nothing else would ever notice it had gone stale.
        void ForgetCamera(const Camera* camera) const { if (_currentCamera == camera) _currentCamera = nullptr; }

        glm::vec2 ConvertWorldToScreen(glm::vec2 worldPos) const;
        glm::vec2 ConvertScreenToWorld(glm::vec2 screenPos) const;

    private:
        void ResetActiveCamera(const void* p);
        
    private:
        mutable Camera* _currentCamera = nullptr;
        
    };

    inline CameraManager& GetCameraManager()
    {
        return CameraManager::GetInstance();
    }
}
