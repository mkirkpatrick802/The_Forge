#include "Renderer.h"

#include "System.h"

Engine::Renderer::Renderer(): _context(nullptr)
{
	CreateRenderer();
}

void Engine::Renderer::CreateRenderer()
{
	auto window = System::GetWindow();
	_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, _context);
	SDL_GL_SetSwapInterval(1);
}

void Engine::Renderer::Render() const
{

}

Engine::Renderer::~Renderer() = default;
