#pragma once
#include <map>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "shader.h"

namespace Engine
{
    struct Character
    {
        unsigned int textureID;  // Texture ID for the glyph
        glm::vec2 size;         // Width and height of glyph
        glm::vec2 bearing;      // Offset from baseline
        unsigned int advance;    // Horizontal offset to next glyph
    };
    
    class Font
    {
    public:
        Font(const std::string& fontPath, unsigned int fontSize);
        void RenderText(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color);

        void SetFontSize(unsigned int fontSize);
    private:
        void LoadFont(const std::string& fontPath, unsigned int fontSize);
        void SetupRendering();
        
    private:
        std::map<char, Character> _characters;
        unsigned int _vao, _vbo;
        Shader _shader;
        std::string _fontPath;
        int _fontSize;
    
    };
}
