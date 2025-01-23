#include "Renderer.h"
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraManager.h"
#include "Framebuffer.h"
#include "PixelGrid.h"
#include "UIManager.h"
#include "Engine/EventData.h"
#include "Engine/EventSystem.h"

Engine::Context Engine::Renderer::_context = nullptr;

Engine::Renderer::Renderer()
{
	CreateRenderer();
	
	UIManager::Init();

	EventSystem::GetInstance()->RegisterEvent(ED_CreateComponent::GetEventName(), this, &Renderer::CreateSpriteRenderer);
	EventSystem::GetInstance()->RegisterEvent(ED_DestroyComponent::GetEventName(), this, &Renderer::DeleteSpriteRenderer);
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

	BufferRegistry::GetRegistry()->AddBuffer(BufferRegistry::BufferType::SCENE, std::make_shared<Framebuffer>(ReferenceResolution, false));
	_grid = std::make_unique<PixelGrid>();
}

void Engine::Renderer::CreateSpriteRenderer(const void* data)
{
	const auto eventData = static_cast<const ED_CreateComponent*>(data);
	if (!eventData || eventData->componentID != SPRITE_RENDERER) return;

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
	const auto eventData = static_cast<const ED_DestroyComponent*>(data);
	if (!eventData) return;
	
	auto spriteRenderer = dynamic_cast<SpriteRenderer*>(eventData->component);
	if (spriteRenderer == nullptr) return;
	
	_spriteRendererPool.Delete(spriteRenderer);
	std::erase_if(_renderList,
	              [spriteRenderer, eventData](const std::pair<int16_t, SpriteRenderer*>& entry) {
		              return entry.second == spriteRenderer;
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
	const auto sceneFBO = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
	{
		sceneFBO->Bind();
		
		glViewport(0, 0, (int)sceneFBO->GetSize().x, (int)sceneFBO->GetSize().y);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);

		_grid->Render();
		
		if (!_renderList.empty())
			for (const auto& val : _renderList | std::views::values)
				val->DrawSprite();

		sceneFBO->Unbind();
	}
	
	UIManager::RenderWindows();
	UIManager::FinishUIRender();
	
	sceneFBO->CheckResize();

	SDL_GL_SwapWindow(System::GetWindow());
}

Engine::Renderer::~Renderer()
{
	_renderList.clear();
	_renderList.shrink_to_fit();
	
	EventSystem::GetInstance()->DeregisterEvent(ED_CreateComponent::GetEventName());
	EventSystem::GetInstance()->DeregisterEvent(ED_DestroyComponent::GetEventName());
	
	UIManager::CleanUp();
	BufferRegistry::GetRegistry()->CleanUp();
	CameraManager::CleanUp();
	
	SDL_GL_DeleteContext(_context);
}

Vector2D Engine::Renderer::ConvertWorldToScreen(Vector2D worldPos)
{
	const auto screenLocation = Vector2D(worldPos.x + System::GetWindowSize().x / 2, worldPos.y + System::GetWindowSize().y / 2);
	return screenLocation;
}
