#pragma once
#include <SDL_render.h>
#include <vector>

#include "Data.h"

namespace Engine
{
	typedef SDL_GLContext Context;

	class SpriteRenderer;

	typedef std::vector<std::pair<int16, SpriteRenderer*>*> RenderListPair;
	class Renderer
	{
	public:

		Renderer();

		void Render() const;

		static Context* GetContext() { return &_context; }

		~Renderer();

	private:

		void CreateRenderer();

	private:

		static Context _context;
		RenderListPair _renderList;

	};
}
