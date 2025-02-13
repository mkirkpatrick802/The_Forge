#include "TextureLoader.h"
#include "Engine/System.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unique_ptr<Engine::Texture> Engine::CreateTexture(const std::string& filepath, Texture::TextureType type)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!image) 
    {
        System::GetInstance().LogToErrorFile("Failed to Load Texture in Texture Loader");
        return nullptr;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    stbi_image_free(image);

    auto size = glm::vec2(width, height);
    auto texture = std::make_unique<Texture>(textureID, filepath, type, size);
    return std::move(texture);
}

std::unique_ptr<Engine::Texture> Engine::CreateTexture(const glm::vec2 size, Texture::TextureType type)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)size.x, (int)size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    auto texture = std::make_unique<Texture>(textureID, type, size);
    return std::move(texture);
}