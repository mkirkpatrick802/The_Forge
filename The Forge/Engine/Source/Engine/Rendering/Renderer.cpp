#include "Renderer.h"
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraHelper.h"
#include "CameraManager.h"
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

	// Generate the VAO and VBO for line rendering
	glGenVertexArrays(1, &_lineVAO);
	glGenBuffers(1, &_lineVBO);

	// Bind the VAO
	glBindVertexArray(_lineVAO);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);

	// Set up the vertex data (we'll update this data every time we draw a line)
	// Currently, there's no need to load data into the VBO yet

	// Specify the layout of the vertex data
	// Position attribute (x, y)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Unbind the VAO and VBO after setting up the layout
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	_lineShader.Compile("Assets/Engine Assets/Shaders/Line.vert", "Assets/Engine Assets/Shaders/Line.frag");
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

			RemoveComponentFromRenderList(renderable);
		}
	
	_renderList.emplace_back(pair);
	
	SortRenderList();
}

void Engine::Renderer::RemoveComponentFromRenderList(IRenderable* renderable)
{
	if (renderable == nullptr) return;

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
		
		glViewport(0, 0, (int)sceneFBO->GetSize().x, (int)sceneFBO->GetSize().y);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);

		//_grid->Render();
		
		if (!_renderList.empty())
			for (const auto& val : _renderList | std::views::values)
			{
				if (auto go = val->GetGameObject())
				{
					auto data = ShaderUniformSystem::CollectUniforms(go);
					val->Render(data);
				}
			}

		if (!GetEngineManager().IsEditorEnabled())
			GetCollisionManager().DebugRender();
		
		// Render all the lines stored in the _debugLines vector
		for (const auto& line : _debugLines)
		{
			GLfloat vertices[] = {
				line.start.x, line.start.y, // Start point
				line.end.x, line.end.y		// End point
			};

			// Bind the VAO and VBO for the line
			glBindVertexArray(_lineVAO);
			glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			// Set up the shader program
			_lineShader.Use();
			_lineShader.SetVector4f("inColor", glm::vec4(line.color, 1));
			_lineShader.SetMatrix4("projection", GetProjectionMatrix());
			_lineShader.SetMatrix4("view", GetViewMatrix());

			// Specify the layout of the vertex data (position)
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Draw the line using GL_LINES
			glDrawArrays(GL_LINES, 0, 2);

			// Unbind VAO and VBO
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// Clear the lines after rendering, if you want to render them only once per frame
		_debugLines.clear();

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

void Engine::Renderer::RenderLine(const glm::vec2 start, const glm::vec2 end, const glm::vec3 color)
{
	const glm::vec2 screenStart = GetCameraManager().ConvertWorldToScreen(glm::vec2(start.x, -start.y));
	const glm::vec2 screenEnd = GetCameraManager().ConvertWorldToScreen(glm::vec2(end.x, -end.y));
	_debugLines.push_back({screenStart, screenEnd, color});
}

void Engine::Renderer::ClearLines()
{
	_debugLines.clear();
}

Engine::Renderer::~Renderer()
{
	_renderList.clear();
	_renderList.shrink_to_fit();
	
	UIManager::CleanUp();
	BufferRegistry::GetRegistry()->CleanUp();
	
	SDL_GL_DeleteContext(_context);
}
