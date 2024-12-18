#pragma once
#include <glad/glad.h>
#include "System.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine
{
	inline GLuint LoadTexture(const char* filename, Vector2D& size)
	{
        int width, height, channels;
        unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
        if (!image) 
        {
            System::LogToErrorFile("Failed to Load Texture in Texture Loader");
            return 0;
        }

        size.x = (float)width;
        size.y = (float)height;

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(image);

        return textureID;
    }
}
