#pragma once
#include <SDL_render.h>
#include <vector>

#include "Engine/ComponentPool.h"
#include "Engine/Data.h"

namespace Engine
{
	class Framebuffer;
	class SpriteRenderer;
	typedef SDL_GLContext Context;

	typedef std::vector<std::pair<int16, SpriteRenderer*>> RenderListPair;
	class Renderer
	{
	public:
		Renderer();

		void Render() const;
		
		static Context* GetContext() { return &_context; }

		//TODO Make this a global function
		static Vector2D ConvertWorldToScreen(Vector2D worldPos);
		
		~Renderer();

	private:

		void CreateRenderer();
		void CreateSpriteRenderer(const void* data);
		void DeleteSpriteRenderer(const void* data);
		
		void SortRenderList();
  
	private:
		
		static Context _context;
		
		RenderListPair _renderList;
		ComponentPool<SpriteRenderer> _spriteRendererPool;

	};
}
