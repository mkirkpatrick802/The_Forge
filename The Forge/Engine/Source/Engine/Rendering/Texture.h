#pragma once
#include <string>
#include <glm/vec2.hpp>

#include "Engine/System.h"
#include "glad/glad.h"

namespace Engine
{
    // Call a static function from the texture loader that returns a smart pointer to a texture object.
    class Texture
    {
    public:
        
        enum class TextureType { PIXEL = 0, FRAMEBUFFER };
        
        Texture(GLuint ID, const std::string& filepath, TextureType type, glm::vec2 size);
        Texture(GLuint ID, TextureType type, glm::vec2 size);
        ~Texture() = default;

    public:
        GLuint GetID() const { if (_ID == -1) {System::GetInstance().DisplayMessageBox("Error", "Failed to get Texture ID"); exit(1);} return _ID; }
        glm::vec2 GetSize() const { return _size; }
        std::string GetFilePath() const { return _filepath; }
        
    private:

        GLuint _ID = -1;
        std::string _filepath;
        glm::vec2 _size;
        TextureType _type;
    };
}
