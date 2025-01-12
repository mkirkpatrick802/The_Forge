#pragma once
#include <glad/glad.h>
#include "Data.h"

namespace Engine
{
	GLuint LoadTexture(const char* filename, Vector2D& size);
}
