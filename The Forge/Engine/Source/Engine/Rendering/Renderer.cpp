﻿#include "Renderer.h"
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraHelper.h"
#include "CameraManager.h"
#include "DebugRenderer.h"
#include "Framebuffer.h"
#include "PixelGrid.h"
#include "ShaderUniformSystem.h"
#include "UIManager.h"
#include "Engine/EngineManager.h"
#include "Engine/EventSystem.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/SpriteRenderer.h"
#include "Engine/Components/TextRenderer.h"

Engine::Context Engine::Renderer::_context = nullptr;
Engine::Renderer::Renderer()
{
	CreateRenderer();
	UIManager::Init();
}

void Engine::Renderer::CreateRenderer()
{
	const auto window = GetAppWindow();
	if (window == nullptr) return;
	
	_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, _context);
	SDL_GL_SetSwapInterval(1);
	
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		System::GetInstance().DisplayMessageBox("Failed to Load GLAD", "gladLoadGLLoader has failed us");
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
	
	_quadShader.Compile("Assets/Engine Assets/Shaders/ScreenQuad.vert", "Assets/Engine Assets/Shaders/ScreenQuad.frag");
}

void Engine::Renderer::AddComponentToRenderList(IRenderable* renderable)
{
	if (renderable == nullptr) return;
	const auto pair = std::pair(renderable->GetSortingLayer(), renderable);
	for (const auto val : _renderList) // This is gross
		if (val.second == renderable)
		{
			if (val == pair)
			{
				return;
			}

			RemoveComponentFromRenderList(val.second);
		}
	
	_renderList.emplace_back(pair);
	
	SortRenderList();
}

void Engine::Renderer::RemoveComponentFromRenderList(IRenderable* renderable)
{
	if (renderable == nullptr) return;
	if (APPLICATION_CLOSING) return;
	if (_renderList.empty()) return;
	
	std::erase_if(_renderList,
				  [renderable](const std::pair<int16_t, IRenderable*>& sprite) {
						  return sprite.second == renderable;
				  });
	
	SortRenderList();
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
		
		glViewport(0, 0, static_cast<int>(sceneFBO->GetSize().x), static_cast<int>(sceneFBO->GetSize().y));
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
		
		if (!_renderList.empty())
			for (const auto& val : _renderList | std::views::values)
			{
				if (const auto go = val->GetGameObject())
				{
					auto data = ShaderUniformSystem::CollectUniforms(go);
					val->Render(data);
				}
			}

		GetDebugRenderer().Render();

		sceneFBO->Unbind();
	}

	if (!GetEngineManager().IsEditorEnabled())
	{
		sceneFBO->Resize(GetAppWindowSize());
		
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

	SDL_GL_SwapWindow(GetAppWindow());
}

Engine::Renderer::~Renderer()
{
	UIManager::CleanUp();
	BufferRegistry::GetRegistry()->CleanUp();
	
	SDL_GL_DeleteContext(_context);
}
