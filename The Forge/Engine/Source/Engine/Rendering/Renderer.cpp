#include "Renderer.h"
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraManager.h"
#include "Framebuffer.h"
#include "PixelGrid.h"
#include "UIManager.h"
#include "Engine/EngineManager.h"
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

	float quadVertices[] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,   0.0f, 1.0f, // Top-left
		-1.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
		 1.0f,  1.0f,   1.0f, 1.0f, // Bottom-right
		 1.0f, -1.0f,   1.0f, 0.0f  // Top-right
	};

	glGenVertexArrays(1, &_quadVAO);
	glGenBuffers(1, &_quadVBO);
	
	glBindVertexArray(_quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	String vertex = "Assets/Shaders/ScreenQuad.vert";
	String fragment = "Assets/Shaders/ScreenQuad.frag";

	_quadShader.Compile(vertex.c_str(), fragment.c_str());
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

void Engine::Renderer::Render()
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

	if (!EngineManager::IsEditorEnabled())
	{
		sceneFBO->Resize(System::GetWindowSize());
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Render scene FBO to screen quad
		_quadShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneFBO->GetTextureID());
		_quadShader.SetInteger("screenTexture", 0);

		glBindVertexArray(_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	
	UIManager::RenderWindows();
	
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
