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

		~Renderer();

	private:

		void CreateRenderer();

	private:

		Context _context;
		RenderListPair _renderList;

	};
}
