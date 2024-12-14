#pragma once
#include "TextureLoader.h"


namespace Engine
{
    class Texture
    {
    public:

        Texture()
        {
            ID = -1;
            size = Vector2D(0, 0);
        }
        
        explicit Texture(const char* filename)
        {
            ID = LoadTexture(filename, size);
        }

        unsigned int ID;

    private:

        Vector2D size;
    };
}
