#pragma once
#include <memory>

#include "Texture.h"
#include "Engine/Data.h"

namespace Engine
{
	std::shared_ptr<Texture> CreateTexture(const String& filepath, Texture::TextureType type = Texture::TextureType::PIXEL);
	std::shared_ptr<Texture> CreateTexture(Vector2D size, Texture::TextureType type = Texture::TextureType::FRAMEBUFFER);
}
