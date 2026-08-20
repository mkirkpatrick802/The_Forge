#pragma once
#include <map>
#include <string>
#include <glm/mat4x4.hpp>
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

        // Draws through the scene camera's projection. What every existing caller wants:
        // a TextRenderer component is part of the world being viewed.
        void RenderText(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color);

        // Draws through a projection the caller supplies.
        //
        // Exists for the UI layer, which has no camera to borrow one from. The overload
        // above resolves to CameraHelper::GetProjectionMatrix(), and that returns
        // *identity* when no Camera component is active -- which is exactly the state a
        // main menu is in, since Camera rides on the player prefab. Text drawn that way
        // lands in raw normalised device coordinates and is, in practice, invisible.
        //
        // pos.y is the baseline, and the glyph quads are wound for a y-down projection.
        void RenderText(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color, const glm::mat4& projection,
                        float opacity = 1.0f);

        void SetFontSize(unsigned int fontSize);

        // The advance width and line height the given string would occupy, in pixels at
        // the font's loaded size. Needed to centre or right-align a label without
        // drawing it first.
        glm::vec2 MeasureText(const std::string& text, float scale = 1.0f) const;

        // Distance from the top of the tallest loaded glyph down to the baseline. A
        // caller positioning text by its top-left corner adds this to get the baseline
        // RenderText expects.
        float GetAscent(float scale = 1.0f) const;

        int GetFontSize() const { return _fontSize; }

    private:
        void LoadFont(const std::string& fontPath, unsigned int fontSize);
        void SetupRendering();
        
    private:
        std::map<char, Character> _characters;
        unsigned int _vao, _vbo;
        Shader _shader;
        std::string _fontPath;
        int _fontSize;

        // The tallest bearing across the loaded glyphs, cached at load. Derived from the
        // glyphs rather than from FreeType's face metrics so it matches what actually
        // gets drawn.
        float _ascent = 0.0f;
    
    };
}
