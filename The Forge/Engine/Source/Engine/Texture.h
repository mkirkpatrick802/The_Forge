#pragma once
#include "TextureLoader.h"

namespace Engine
{
    class Texture
    {
    public:

        Texture(): ID(-1), _size()
        {
             
        }

        explicit Texture(const char* filename)
        {
            _filepath = filename;
            ID = LoadTexture(filename, _size);
        }

        ~Texture()
        {
            
        }
        
        String GetFilePath() const { return _filepath; }
        
        unsigned int ID;
    
    private:

        Vector2D _size;
        String _filepath;
    };
}
