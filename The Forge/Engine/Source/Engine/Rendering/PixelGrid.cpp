#include "PixelGrid.h"

#include "BufferRegistry.h"
#include "CameraHelper.h"
#include "CameraManager.h"
#include "Engine/System.h"

Engine::PixelGrid::PixelGrid()
{
    std::string vertex = "Assets/Engine Assets/Shaders/PixelGrid.vert";
    std::string fragment = "Assets/Engine Assets/Shaders/PixelGrid.frag";

    _shader.Compile(vertex.c_str(), fragment.c_str());

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 2 * sizeof(int), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

Engine::PixelGrid::~PixelGrid()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void Engine::PixelGrid::Render()
{
    const auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);

    _shader.Use();
    _shader.SetMatrix4("projection", GetProjectionMatrix());
    _shader.SetMatrix4("view", GetViewMatrix());

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    
    // Loop to draw grid lines:
    //for (int x = (int)-sceneFBO->GetWindowSize().x / 2; x <= (int)sceneFBO->GetWindowSize().x / 2; x += _gridSize)
    {
        int lineVertices[] = {
            0, (int)GetAppWindowSize().y / 2,  // Start point of the line
            (int)GetAppWindowSize().x, (int)GetAppWindowSize().y / 2    // End point of the line
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
        glDrawArrays(GL_LINES, 0, 2);  // Draw vertical line
    }

    //for (int y = (int)-sceneFBO->GetWindowSize().y / 2; y <= (int)sceneFBO->GetWindowSize().y / 2; y += _gridSize)
    {
        int lineVertices[] = {
            (int)GetAppWindowSize().x / 2, 0,  // Start point of the line
            (int)GetAppWindowSize().x / 2, (int)GetAppWindowSize().y    // End point of the line
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
        glDrawArrays(GL_LINES, 0, 2);  // Draw horizontal line
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
