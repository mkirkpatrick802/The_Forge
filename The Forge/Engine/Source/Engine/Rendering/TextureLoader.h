#pragma once
#include <memory>
#include "Texture.h"

namespace Engine
{
	std::unique_ptr<Texture> CreateTexture(const std::string& filepath, Texture::TextureType type = Texture::TextureType::PIXEL);
	std::unique_ptr<Texture> CreateTexture(glm::vec2 size, Texture::TextureType type = Texture::TextureType::FRAMEBUFFER);
}
