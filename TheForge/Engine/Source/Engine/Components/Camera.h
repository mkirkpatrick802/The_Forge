#pragma once
#include "Component.h"
#include "ComponentUtils.h"
#include "glm/glm.hpp"

namespace Engine
{
    struct CameraBounds
    {
        float left, top, right, bottom;
    };
    
    class Camera final : public Component
    {
    public:
        
        Camera();
        void OnActivation() override;

        void SetZoom(const float zoom) { _zoom = glm::clamp(zoom, _zoomBounds.x, _zoomBounds.y); }  // Clamping for stability
        void ZoomIn(const float amount) { SetZoom(_zoom - amount); }
        void ZoomOut(const float amount) { SetZoom(_zoom + amount); }
        
    private:
        CameraBounds _bounds;
        float _zoom = 1;
        glm::vec2 _zoomBounds;
        glm::mat4 _projection;
        glm::mat4 _view;
        glm::vec2 _refResolution = glm::vec2(320, 240);

    public:
        glm::vec2 GetReferenceResolution() const { return _refResolution; } 
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();
        CameraBounds GetBounds() const { return _bounds; }
        float GetZoom() const { return _zoom; }

        // X is min, Y is max
        void SetZoomBounds(const glm::vec2 zoomBounds) { _zoomBounds = zoomBounds; }
    };
    
    REGISTER_COMPONENT(Camera)
}
