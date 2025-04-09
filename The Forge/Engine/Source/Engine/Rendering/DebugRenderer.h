#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

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
        
    private:
        void Render();

    private:
        struct DebugLine
        {
            glm::vec2 start, end;
            glm::vec3 color;
            DebugLine(const glm::vec2& s, const glm::vec2& e, const glm::vec3& c) : start(s), end(e), color(c) {}
        };

        std::vector<DebugLine> _lines;
        
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
