#pragma once
#include <memory>
#include <SDL_render.h>
#include <vector>

#include "Shader.h"
#include "glad/glad.h"

namespace Engine
{
	class Component;
	class Framebuffer;
	class PixelGrid;
	typedef SDL_GLContext Context;

	const auto ReferenceResolution = glm::vec2(320, 180);
	
	typedef std::vector<std::pair<int16_t, Component*>> RenderList;
	class Renderer
	{
	public:
		
		Renderer();
		~Renderer();
		
		void Render();

		void AddComponentToRenderList(Component* spriteRenderer);
		void RemoveComponentFromRenderList(Component* spriteRenderer);
		
	private:

		void CreateRenderer();
		void SortRenderList();
  
	private:
		
		static Context _context;

		GLuint _quadVAO, _quadVBO;
		Shader _quadShader;
		
		RenderList _renderList;

		std::unique_ptr<PixelGrid> _grid;

	public:
		
		static Context* GetContext() { return &_context; }
		
	};
}
