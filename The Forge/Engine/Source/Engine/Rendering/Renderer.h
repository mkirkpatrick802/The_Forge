#pragma once
#include <memory>
#include <SDL_render.h>
#include <vector>

#include "Shader.h"
#include "glad/glad.h"

namespace Engine
{
	class Framebuffer;
	class SpriteRenderer;
	class PixelGrid;
	typedef SDL_GLContext Context;

	const glm::vec2 ReferenceResolution = glm::vec2(320, 180);
	
	typedef std::vector<std::pair<int16_t, SpriteRenderer*>> RenderListPair;
	class Renderer
	{
	public:
		Renderer();

		void Render();
		
		static Context* GetContext() { return &_context; }
		
		~Renderer();

		static glm::vec2 ConvertWorldToScreen(glm::vec2 worldPos);
		
	private:

		void CreateRenderer();
		void CreateSpriteRenderer(const void* data);
		void DeleteSpriteRenderer(const void* data);
		
		void SortRenderList();
  
	private:
		
		static Context _context;

		GLuint _quadVAO, _quadVBO;
		Shader _quadShader;
		
		RenderListPair _renderList;

		std::unique_ptr<PixelGrid> _grid;
	};
}
