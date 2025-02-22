#include "Font.h"

#include <iostream>
#include <ostream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

#include "CameraHelper.h"
#include "glad/glad.h"

Engine::Font::Font(const std::string& fontPath, unsigned int fontSize) : _fontPath(fontPath), _fontSize(fontSize)
{
    LoadFont(fontPath, fontSize);
    SetupRendering();
}

void Engine::Font::LoadFont(const std::string& fontPath, unsigned int fontSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << '\n';
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << '\n';
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load Glyph: " << c << '\n';
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 
                     face->glyph->bitmap.width, face->glyph->bitmap.rows, 
                     0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        _characters[c] = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Engine::Font::SetupRendering()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const std::string vertex = "Assets/Engine Assets/Shaders/Text.vert";
    const std::string fragment = "Assets/Engine Assets/Shaders/Text.frag";
    _shader.Compile(vertex.c_str(), fragment.c_str());
}

void Engine::Font::SetFontSize(unsigned int fontSize)
{
    // Clear existing character textures
    for (auto& [c, charData] : _characters) {
        glDeleteTextures(1, &charData.textureID);
    }
    _characters.clear(); // Remove old character data

    // Reload the font with the new size
    LoadFont(_fontPath, fontSize);
}

void Engine::Font::RenderText(const std::string& text, glm::vec2 pos, float scale, glm::vec3 color)
{
    _shader.Use();
    _shader.SetVector3f("textColor", color);
    _shader.SetMatrix4("projection", GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    for (char c : text) {
        Character ch = _characters[c];
        
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        float xpos = pos.x + ch.bearing.x * scale;
        float ypos = pos.y - (ch.bearing.y * scale);
        
        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 1.0f }, 
            { xpos,     ypos,     0.0f, 0.0f }, 
            { xpos + w, ypos,     1.0f, 0.0f }, 

            { xpos,     ypos + h, 0.0f, 1.0f }, 
            { xpos + w, ypos,     1.0f, 0.0f }, 
            { xpos + w, ypos + h, 1.0f, 1.0f }  
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        pos.x += (ch.advance >> 6) * scale; // Advance to next glyph
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}