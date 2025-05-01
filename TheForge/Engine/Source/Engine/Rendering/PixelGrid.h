#pragma once
#include "shader.h"

namespace Engine
{
    class PixelGrid
    {
    public:
        
        PixelGrid();
        ~PixelGrid();
        
        void Render();

    private:
        
        int _gridSize = 32; // In Pixels

        GLuint _vao, _vbo;
        Shader _shader;
    };
}
