#include "DebugRenderer.h"
#include <memory>
#include <glm/gtc/constants.hpp>

#include "CameraHelper.h"
#include "CameraManager.h"
#include "Font.h"
#include "Engine/CommandRegistry.h"
#include "Engine/CommandUtils.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"
#include "Engine/Time.h"

Engine::DebugRenderer& Engine::DebugRenderer::GetInstance()
{
    static auto instance = std::make_unique<DebugRenderer>();
    return *instance;
}

Engine::DebugRenderer::DebugRenderer()
{
    // Debug visualisation is pure presentation. Headless has no GL context, and
    // this is a lazily-constructed singleton, so the first debug draw would
    // otherwise build its GL resources here and crash.
    if (GetLaunchOptions().headless) return;

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
    CommandRegistry::RegisterCommand("/debug", [this](const std::string& args)
    {
        // Bare "/debug" toggles -- that is what anyone typing it actually wants, and it
        // used to throw std::invalid_argument straight through the ImGui frame.
        if (args.empty())
        {
            enabled = !enabled;
            DEBUG_LOG("Debug rendering %s.", enabled ? "on" : "off")
            return;
        }

        if (bool value; CommandUtils::TryParseBoolean(args, value))
        {
            enabled = value;
            DEBUG_LOG("Debug rendering %s.", enabled ? "on" : "off")
            return;
        }

        DEBUG_LOG("usage: /debug [true|false]")
    });
}

Engine::DebugRenderer::~DebugRenderer()
{
    CommandRegistry::UnregisterCommand("/debug");
}

void Engine::DebugRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color)
{
    // Nothing consumes these when headless, and they would accumulate forever.
    if (GetLaunchOptions().headless) return;

    auto& target = _buildingOverlay ? _overlayLines : _lines;
    target.emplace_back(GetCameraManager().ConvertWorldToScreen(glm::vec2(start.x, -start.y)), GetCameraManager().ConvertWorldToScreen(glm::vec2(end.x, -end.y)), color);
}

void Engine::DebugRenderer::DrawCircle(const glm::vec2& center, const float radius, const glm::vec3& color, const int segments)
{
    if (GetLaunchOptions().headless) return;

    const float angleStep = glm::two_pi<float>() / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i)
    {
        const float angle1 = static_cast<float>(i) * angleStep;
        const float angle2 = (static_cast<float>(i) + 1) * angleStep;

        glm::vec2 point1 = center + glm::vec2(cos(angle1) * radius, sin(angle1) * radius);
        glm::vec2 point2 = center + glm::vec2(cos(angle2) * radius, sin(angle2) * radius);

        // Through DrawLine, so _buildingOverlay is honoured. Emplacing into _lines here
        // meant DrawOverlayCircle put its output in the list that is only drawn when
        // debug rendering is on -- which is the one thing an overlay must not depend on.
        DrawLine(point1, point2, color);
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

void Engine::DebugRenderer::DrawPolygon(const std::vector<glm::vec2>& points, const glm::vec3& color)
{
    if (points.size() < 2) return;

    for (size_t i = 0; i < points.size(); ++i)
        DrawLine(points[i], points[(i + 1) % points.size()], color);
}

void Engine::DebugRenderer::DrawOverlayCircle(const glm::vec2& center, const float radius, const glm::vec3& color, const int segments)
{
    _buildingOverlay = true;
    DrawCircle(center, radius, color, segments);
    _buildingOverlay = false;
}

void Engine::DebugRenderer::DrawOverlayRectangle(const glm::vec2& center, const glm::vec2& size, const glm::vec3& color)
{
    _buildingOverlay = true;
    DrawRectangle(center, size, color);
    _buildingOverlay = false;
}

void Engine::DebugRenderer::DrawOverlayPolygon(const std::vector<glm::vec2>& points, const glm::vec3& color)
{
    _buildingOverlay = true;
    DrawPolygon(points, color);
    _buildingOverlay = false;
}

void Engine::DebugRenderer::Render()
{
    if (GetLaunchOptions().headless) return;

    if (enabled)
    {
        std::stringstream fps;
        fps << "FPS: ";
        fps << 1 / Time::GetDeltaTime();
        _font->RenderText(fps.str(), glm::vec2(5, 40), 1, glm::vec3(0, 1, 0));

        RenderLines(_lines);
    }

    // Overlays are drawn regardless -- see DrawOverlayCircle.
    RenderLines(_overlayLines);

    // Cleared even when nothing was drawn. Previously this function returned early on
    // !enabled *before* clearing, so anything still calling DrawLine with debug off
    // grew the list forever.
    _lines.clear();
    _overlayLines.clear();
}

void Engine::DebugRenderer::RenderLines(const std::vector<DebugLine>& lines)
{
    for (const auto& line : lines)
    {
        GLfloat vertices[] = {
            line.start.x, line.start.y, // Start point
            line.end.x, line.end.y		// End point
        };

        glBindVertexArray(_lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        _lineShader.Use();
        _lineShader.SetVector4f("inColor", glm::vec4(line.color, 1));
        _lineShader.SetMatrix4("projection", GetProjectionMatrix());
        _lineShader.SetMatrix4("view", GetViewMatrix());

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), static_cast<GLvoid*>(nullptr));
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_LINES, 0, 2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
