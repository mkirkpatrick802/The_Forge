#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "shader.h"
#include "glad/glad.h"

namespace Engine
{
    class Font;

    class DebugRenderer
    {
        friend class Renderer;
    public:
        static DebugRenderer& GetInstance();
        DebugRenderer();
        ~DebugRenderer();
        
        void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);
        void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color, int segments = 16);
        void DrawRectangle(const glm::vec2& center, const glm::vec2& size, const glm::vec3& color);

        // A closed loop of world-space points. Fewer than two points draws nothing.
        void DrawPolygon(const std::vector<glm::vec2>& points, const glm::vec3& color);

        // Drawn whether or not /debug is on.
        //
        // Editor overlays -- the collider outline on the selected object, say -- are a
        // selection aid rather than a debug visualisation, so they should not require
        // remembering to switch debug rendering on first.
        void DrawOverlayCircle(const glm::vec2& center, float radius, const glm::vec3& color, int segments = 16);
        void DrawOverlayRectangle(const glm::vec2& center, const glm::vec2& size, const glm::vec3& color);
        void DrawOverlayPolygon(const std::vector<glm::vec2>& points, const glm::vec3& color);

    private:
        struct DebugLine;
        void Render();
        void RenderLines(const std::vector<DebugLine>& lines);

    private:
        struct DebugLine
        {
            glm::vec2 start, end;
            glm::vec3 color;
            DebugLine(const glm::vec2& s, const glm::vec2& e, const glm::vec3& c) : start(s), end(e), color(c) {}
        };

        std::vector<DebugLine> _lines;
        std::vector<DebugLine> _overlayLines;

        // Set while an overlay shape is being decomposed into lines, so the shape
        // helpers can be reused rather than duplicated per destination.
        bool _buildingOverlay = false;
        
        std::unique_ptr<Font> _font;
        GLuint _lineVAO, _lineVBO;
        Shader _lineShader;
        bool enabled = false;
    };

    inline DebugRenderer& GetDebugRenderer()
    {
        return DebugRenderer::GetInstance();
    }
}
