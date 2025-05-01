#include "DebugRenderer.h"
#include <memory>
#include <glm/gtc/constants.hpp>

#include "CameraHelper.h"
#include "CameraManager.h"
#include "Font.h"
#include "Engine/CommandRegistry.h"
#include "Engine/CommandUtils.h"
#include "Engine/Time.h"

Engine::DebugRenderer& Engine::DebugRenderer::GetInstance()
{
    static auto instance = std::make_unique<DebugRenderer>();
    return *instance;
}

Engine::DebugRenderer::DebugRenderer()
{
    // Generate the VAO and VBO for line rendering
    glGenVertexArrays(1, &_lineVAO);
    glGenBuffers(1, &_lineVBO);

    // Bind the VAO
    glBindVertexArray(_lineVAO);

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);

    // Set up the vertex data (we'll update this data every time we draw a line)
    // Currently, there's no need to load data into the VBO yet

    // Specify the layout of the vertex data
    // Position attribute (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), static_cast<GLvoid*>(nullptr));
    glEnableVertexAttribArray(0);

    // Unbind the VAO and VBO after setting up the layout
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    _lineShader.Compile("Assets/Engine Assets/Shaders/Line.vert", "Assets/Engine Assets/Shaders/Line.frag");
    _font = std::make_unique<Font>("Assets/Engine Assets/Fonts/Consolas.ttf", 15);

    // Commands
    CommandRegistry::RegisterCommand("/debug", [this](const std::string& args){ enabled = CommandUtils::ParseBoolean(args); });
}

Engine::DebugRenderer::~DebugRenderer()
{
    CommandRegistry::UnregisterCommand("/debug");
}

void Engine::DebugRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color)
{
    _lines.emplace_back(GetCameraManager().ConvertWorldToScreen(glm::vec2(start.x, -start.y)), GetCameraManager().ConvertWorldToScreen(glm::vec2(end.x, -end.y)), color);
}

void Engine::DebugRenderer::DrawCircle(const glm::vec2& center, const float radius, const glm::vec3& color, const int segments)
{
    const float angleStep = glm::two_pi<float>() / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i)
    {
        const float angle1 = static_cast<float>(i) * angleStep;
        const float angle2 = (static_cast<float>(i) + 1) * angleStep;

        glm::vec2 point1 = center + glm::vec2(cos(angle1) * radius, sin(angle1) * radius);
        glm::vec2 point2 = center + glm::vec2(cos(angle2) * radius, sin(angle2) * radius);

        _lines.emplace_back(GetCameraManager().ConvertWorldToScreen(glm::vec2(point1.x, -point1.y)), GetCameraManager().ConvertWorldToScreen(glm::vec2(point2.x, -point2.y)), color);
    }
}

void Engine::DebugRenderer::DrawRectangle(const glm::vec2& center, const glm::vec2& size, const glm::vec3& color)
{
    const glm::vec2 halfSize = size * 0.5f;

    const glm::vec2 topLeft = glm::vec2(center.x - halfSize.x, center.y + halfSize.y);
    const glm::vec2 topRight = glm::vec2(center.x + halfSize.x, center.y + halfSize.y);
    const glm::vec2 bottomLeft = glm::vec2(center.x - halfSize.x, center.y - halfSize.y);
    const glm::vec2 bottomRight = glm::vec2(center.x + halfSize.x, center.y - halfSize.y);

    DrawLine(topLeft, topRight, color);    // Top
    DrawLine(topRight, bottomRight, color); // Right
    DrawLine(bottomRight, bottomLeft, color); // Bottom
    DrawLine(bottomLeft, topLeft, color);  // Left
}

void Engine::DebugRenderer::Render()
{
    if (!enabled) return;

    std::stringstream fps;
    fps << "FPS: ";
    fps << 1 / Time::GetDeltaTime();
    _font->RenderText(fps.str(), glm::vec2(5, 40), 1, glm::vec3(0, 1, 0));
    
    // Render all the lines stored in the _debugLines vector
    for (const auto& line : _lines)
    {
        GLfloat vertices[] = {
            line.start.x, line.start.y, // Start point
            line.end.x, line.end.y		// End point
        };

        // Bind the VAO and VBO for the line
        glBindVertexArray(_lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Set up the shader program
        _lineShader.Use();
        _lineShader.SetVector4f("inColor", glm::vec4(line.color, 1));
        _lineShader.SetMatrix4("projection", GetProjectionMatrix());
        _lineShader.SetMatrix4("view", GetViewMatrix());

        // Specify the layout of the vertex data (position)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), static_cast<GLvoid*>(nullptr));
        glEnableVertexAttribArray(0);

        // Draw the line using GL_LINES
        glDrawArrays(GL_LINES, 0, 2);

        // Unbind VAO and VBO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    _lines.clear(); // Clear for next frame
}
