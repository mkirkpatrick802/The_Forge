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

	constexpr auto ReferenceResolution = glm::vec2(320, 180);
	
	typedef std::vector<std::pair<int16_t, IRenderable*>> RenderList;
	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		
		void Render();

		// Draws the UI layer over a cleared backbuffer and presents it, without touching
		// the scene at all.
		//
		// Exists for the loading screen. Level loading is one blocking call on the main
		// thread, so while it runs the game loop is not turning and Render is not being
		// called -- a loading screen that only drew from the loop would show its first
		// frame after the load it was meant to cover had finished. This is what the
		// loader's progress callback drives instead.
		//
		// Deliberately does not draw the scene: mid-load there is no coherent world to
		// draw, and half a level with no camera is worse than a clean background.
		void PresentUIOnly();

		// Dumps why the screen looks the way it does: framebuffer size, how many
		// renderables are queued, whether there is a camera, and what the matrices came
		// out as. A degenerate projection draws nothing while still clearing, which on
		// screen is indistinguishable from "the scene vanished".
		void ReportRenderState() const;

		void AddComponentToRenderList(IRenderable* renderable);
		void RemoveComponentFromRenderList(IRenderable* renderable);
		
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
