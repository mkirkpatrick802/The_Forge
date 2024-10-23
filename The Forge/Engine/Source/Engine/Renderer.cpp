#include "Renderer.h"

#include "System.h"
#include <glad/glad.h>

#include "UIManager.h"

Engine::Context Engine::Renderer::_context = nullptr;

Engine::Renderer::Renderer()
{
	CreateRenderer();

	UIManager::Init();
}

void Engine::Renderer::CreateRenderer()
{
	const auto window = System::GetWindow();
	_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, _context);
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		System::DisplayMessageBox("Failed to Load GLAD", "gladLoadGLLoader has failed us");
		assert(0);
	}
}

void Engine::Renderer::Render() const
{
	glViewport(0, 0, (int)System::GetWindowSize().x, (int)System::GetWindowSize().y);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	UIManager::RenderWindows();
	UIManager::FinishUIRender();

	SDL_GL_SwapWindow(System::GetWindow());
}

Engine::Renderer::~Renderer()
{
	UIManager::CleanUp();

	SDL_GL_DeleteContext(_context);
}