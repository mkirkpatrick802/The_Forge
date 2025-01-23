#pragma once
#include <SDL_render.h>
#include <vector>

#include "Engine/ComponentPool.h"
#include "Engine/Data.h"

namespace Engine
{
	class Framebuffer;
	class SpriteRenderer;
	class PixelGrid;
	typedef SDL_GLContext Context;

	const Vector2D ReferenceResolution = Vector2D(320, 180);
	
	typedef std::vector<std::pair<int16, SpriteRenderer*>> RenderListPair;
	class Renderer
	{
	public:
		Renderer();

		void Render() const;
		
		static Context* GetContext() { return &_context; }
		
		~Renderer();

		static Vector2D ConvertWorldToScreen(Vector2D worldPos);
		
	private:

		void CreateRenderer();
		void CreateSpriteRenderer(const void* data);
		void DeleteSpriteRenderer(const void* data);
		
		void SortRenderList();
  
	private:
		
		static Context _context;
		
		RenderListPair _renderList;
		ComponentPool<SpriteRenderer> _spriteRendererPool;

		std::unique_ptr<PixelGrid> _grid;
	};
}
