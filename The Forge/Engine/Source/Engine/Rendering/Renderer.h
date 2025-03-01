#pragma once
#include <memory>
#include <SDL_render.h>
#include <vector>

#include "Shader.h"
#include "glad/glad.h"

namespace Engine
{
	class IRenderable;
	class Framebuffer;
	class PixelGrid;
	typedef SDL_GLContext Context;

	struct LineData
	{
		glm::vec2 start;
		glm::vec2 end;
		glm::vec3 color;
	};

	constexpr auto ReferenceResolution = glm::vec2(320, 180);
	
	typedef std::vector<std::pair<int16_t, IRenderable*>> RenderList;
	class Renderer
	{
	public:
		
		Renderer();
		~Renderer();
		
		void Render();
		void RenderLine(glm::vec2 start, glm::vec2 end, glm::vec3 color);
		void ClearLines();

		void AddComponentToRenderList(IRenderable* renderable);
		void RemoveComponentFromRenderList(IRenderable* renderable);
		
	private:

		void CreateRenderer();
		void SortRenderList();
  
	private:
		
		static Context _context;

		GLuint _quadVAO, _quadVBO;
		Shader _quadShader;

		GLuint _lineVAO, _lineVBO;
		Shader _lineShader;

		std::vector<LineData> _debugLines;  // Store the lines to be rendered
		
		RenderList _renderList;

		std::unique_ptr<PixelGrid> _grid;

	public:
		
		static Context* GetContext() { return &_context; }
		
	};
}
