#include "Renderer.h"
#include <SDL_events.h>
#include <cmath>
#include <glad/glad.h>

#include "BufferRegistry.h"
#include "CameraHelper.h"
#include "CameraManager.h"
#include "DebugRenderer.h"
#include "Framebuffer.h"
#include "PixelGrid.h"
#include "ShaderUniformSystem.h"
#include "UIManager.h"
#include "Engine/UI/UIRoot.h"
#include "Engine/CommandRegistry.h"
#include "Engine/EngineManager.h"
#include "Engine/GameObject.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/EventSystem.h"
#include "Engine/LaunchOptions.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/Camera.h"
#include "Engine/Components/PlayerController.h"
#include "Engine/Components/SpriteRenderer.h"
#include "Engine/Components/TextRenderer.h"

Engine::Context Engine::Renderer::_context = nullptr;

Engine::Renderer::Renderer()
{
	CommandRegistry::RegisterCommand("/rendiag", [this](const std::string&) { ReportRenderState(); });

	// The object is always constructed -- renderables register with it through
	// GetRenderer() regardless -- but with no window there is no GL context to set
	// up, and nothing will ever be drawn.
	if (GetLaunchOptions().headless) return;

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

void Engine::Renderer::ReportRenderState() const
{
	const auto fbo = BufferRegistry::GetRegistry()->GetBuffer(BufferRegistry::BufferType::SCENE);
	const auto camera = GetCameraManager().GetActiveCamera();

	const glm::mat4 projection = GetProjectionMatrix();
	const glm::mat4 view = GetViewMatrix();

	DEBUG_LOG("--- render state ---")
	DEBUG_LOG("editor: %s", GetEngineManager().IsEditorEnabled() ? "on" : "off")
	DEBUG_LOG("scene buffer: %.0f x %.0f", fbo->GetSize().x, fbo->GetSize().y)
	DEBUG_LOG("renderables queued: %zu", _renderList.size())

	if (camera != nullptr && camera->gameObject != nullptr)
		DEBUG_LOG("camera: on '%s', zoom %.3f, at (%.0f, %.0f)",
			camera->gameObject->GetName().c_str(), camera->GetZoom(),
			camera->gameObject->GetWorldPosition().x, camera->gameObject->GetWorldPosition().y)
	else
		DEBUG_LOG("camera: NONE -- the world has no viewpoint, so nothing can be placed on screen")

	DEBUG_LOG("projection: x %.6f  y %.6f  w %.3f", projection[0][0], projection[1][1], projection[3][3])
	DEBUG_LOG("view offset: (%.1f, %.1f)", view[3][0], view[3][1])

	// A zero or non-finite scale is the usual reason a lit background shows with nothing
	// on it: every vertex lands on the same point.
	if (projection[0][0] == 0.0f || projection[1][1] == 0.0f)
		DEBUG_LOG("  ^ projection scale is zero -- nothing can draw")
	else if (!std::isfinite(projection[0][0]) || !std::isfinite(projection[1][1]))
		DEBUG_LOG("  ^ projection scale is not finite -- check camera zoom and buffer size")

	int hidden = 0, noObject = 0;
	for (const auto& [layer, renderable] : _renderList)
	{
		if (renderable == nullptr) { ++noObject; continue; }
		if (renderable->GetGameObject() == nullptr) ++noObject;
		else if (renderable->IsHidden()) ++hidden;
	}

	DEBUG_LOG("renderables hidden: %d, detached: %d", hidden, noObject)

	// Where a missing camera usually comes from: no pawn was spawned, so the Camera that
	// rides on the player prefab never existed.
	const auto cameraPool = GetComponentManager().GetPool<Camera>();
	const auto controllerPool = GetComponentManager().GetPool<PlayerController>();

	DEBUG_LOG("camera components: %zu   player controllers: %zu",
		cameraPool ? cameraPool->GetActive().size() : 0,
		controllerPool ? controllerPool->GetActive().size() : 0)

	if (const auto level = LevelManager::GetCurrentLevel())
	{
		int objects = 0, pawns = 0;
		for (const GameObject* go : level->GetAllGameObjects())
		{
			++objects;
			if (go != nullptr && go->GetComponent<PlayerController>() != nullptr) ++pawns;
		}

		DEBUG_LOG("level objects: %d   pawns: %d", objects, pawns)
	}
	else
	{
		DEBUG_LOG("level: NONE")
	}
}

void Engine::Renderer::AddComponentToRenderList(IRenderable* renderable)
{
	if (renderable == nullptr) return;

	// The list is keyed by sorting layer, so a renderable whose layer changed has to be
	// re-inserted under the new one -- calling this again is how that happens.
	const auto pair = std::pair(renderable->GetSortingLayer(), renderable);

	// Found and erased in one pass. The previous version erased from inside a range-for
	// over the same vector and then kept iterating it, which is undefined behaviour.
	const auto existing = std::ranges::find_if(_renderList,
		[renderable](const std::pair<int16_t, IRenderable*>& entry) { return entry.second == renderable; });

	if (existing != _renderList.end())
	{
		if (*existing == pair) return; // already listed under the right layer
		_renderList.erase(existing);
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
	// Nothing is on screen when headless, and SwapWindow would throttle the server
	// loop to the display's refresh rate through vsync.
	if (GetLaunchOptions().headless) return;

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
		
		// Drawing with no camera produces a cleared background and nothing else, which
		// looks exactly like "the scene vanished". Say so once rather than leaving it to
		// be guessed at.
		if (!GetEngineManager().IsEditorEnabled() && GetCameraManager().GetActiveCamera() == nullptr)
		{
			static bool warned = false;
			if (!warned)
			{
				warned = true;
				DEBUG_LOG("Renderer: no active camera -- nothing can be placed on screen. Run /rendiag.")
			}
		}

		if (!_renderList.empty())
			for (const auto& val : _renderList | std::views::values)
			{
				if (const auto go = val->GetGameObject())
				{
					// GPU resources are created here, on first draw, rather than at
					// load -- this is the only place that needs a GL context.
					val->EnsureResourcesResident();

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
	
	// Between the scene and ImGui, deliberately.
	//
	// Above the scene because it is screen-space UI and must not be occluded by the
	// world; below ImGui because ImGui is the editor and the debug console, and those
	// have to stay reachable over the top of whatever the game is showing.
	//
	// Outside the editor branch above: with the editor on, the scene goes into the
	// SceneDisplay panel rather than to the backbuffer, and game UI has no business
	// drawing over the editor's own layout.
	if (!GetEngineManager().IsEditorEnabled())
		UIRoot::Render();

	UIManager::RenderWindows();
	
	sceneFBO->CheckResize();

	SDL_GL_SwapWindow(GetAppWindow());
}

void Engine::Renderer::PresentUIOnly()
{
	if (GetLaunchOptions().headless) return;

	const auto window = GetAppWindow();
	if (window == nullptr) return;

	// Windows marks a window that stops pumping its message queue as "not responding"
	// and paints a ghost over it, which during a long load looks exactly like a hang.
	//
	// PumpEvents and not the input manager's StartProcessInputs: that swaps input
	// buffers and can set APPLICATION_CLOSING, and running it from inside a load would
	// tear the world down halfway through building it.
	SDL_PumpEvents();

	const glm::vec2 windowSize = GetAppWindowSize();
	if (windowSize.x <= 0.0f || windowSize.y <= 0.0f) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	UIRoot::Render();

	SDL_GL_SwapWindow(window);
}

Engine::Renderer::~Renderer()
{
	if (GetLaunchOptions().headless) return;

	// Before the context goes. UIRoot holds shaders, VAOs and font atlases, and a GL
	// object deleted after its context is a crash on some drivers and a leak on the rest.
	UIRoot::CleanUp();

	UIManager::CleanUp();
	BufferRegistry::GetRegistry()->CleanUp();

	SDL_GL_DeleteContext(_context);
}
