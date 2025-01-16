#include "Renderer.h"
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraManager.h"
#include "Framebuffer.h"
#include "UIManager.h"
#include "Engine/EventData.h"
#include "Engine/EventSystem.h"

Engine::Context Engine::Renderer::_context = nullptr;

Engine::Renderer::Renderer()
{
	CreateRenderer();
	
	UIManager::Init();

	EventSystem::GetInstance()->RegisterEvent(ED_CreateSpriteRenderer::GetEventName(), this, &Renderer::CreateSpriteRenderer);
	EventSystem::GetInstance()->RegisterEvent(ED_DestroySpriteRenderer::GetEventName(), this, &Renderer::DeleteSpriteRenderer);
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

	BufferRegistry::GetRegistry()->AddBuffer(BufferRegistry::BufferType::SCENE, std::make_shared<Framebuffer>(Vector2D(320, 240), false));
}

void Engine::Renderer::CreateSpriteRenderer(const void* data)
{
	const ED_CreateSpriteRenderer* eventData = static_cast<const ED_CreateSpriteRenderer*>(data);
	if (!eventData) return;

	// These are very necessary do not get rid of
	Engine::GameObject* go = eventData->gameObject;   
	SpriteRenderer* spriteRenderer = _spriteRendererPool.New(static_cast<Engine::GameObject*>(go));
	spriteRenderer->Init();
	
	if (eventData->data != nullptr)
		spriteRenderer->LoadData(eventData->data);
	
	eventData->gameObject->AddComponent(spriteRenderer);

	const auto pair = std::pair(spriteRenderer->_sortingLayer, spriteRenderer);
	_renderList.push_back(pair);
	
	SortRenderList();
}

void Engine::Renderer::DeleteSpriteRenderer(const void* data)
{
	const auto eventData = static_cast<const ED_DestroySpriteRenderer*>(data);
	if (!eventData) return;
	
	_spriteRendererPool.Delete(eventData->spriteRenderer);
	std::erase_if(_renderList,
	              [eventData](const std::pair<int16_t, SpriteRenderer*>& entry) {
		              return entry.second == eventData->spriteRenderer;
	              });
}

void Engine::Renderer::SortRenderList()
{
	std::ranges::sort(_renderList, [](auto &left, auto &right)
		{
		return left.first < right.first;
	});
}

void Engine::Renderer::Render() const
{
	{
		const auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
		sceneFBO->Bind();
	
		glViewport(0, 0, (int)sceneFBO->GetSize().x, (int)sceneFBO->GetSize().y);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (!_renderList.empty())
			for (const auto& val : _renderList | std::views::values)
				val->DrawSprite();

		sceneFBO->Unbind();	
	}
	
	UIManager::RenderWindows();
	UIManager::FinishUIRender();

	SDL_GL_SwapWindow(System::GetWindow());
}

Engine::Renderer::~Renderer()
{
	_renderList.clear();
	_renderList.shrink_to_fit();
	
	EventSystem::GetInstance()->DeregisterEvent(ED_CreateSpriteRenderer::GetEventName());
	EventSystem::GetInstance()->DeregisterEvent(ED_DestroySpriteRenderer::GetEventName());
	
	UIManager::CleanUp();
	BufferRegistry::GetRegistry()->CleanUp();
	CameraManager::CleanUp();
	
	SDL_GL_DeleteContext(_context);
}
