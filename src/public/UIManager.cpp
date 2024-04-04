
#include "UIManager.h"

#include "Renderer.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

UIManager* UIManager::_instance = nullptr;

void UIManager::Init(const Renderer& renderer)
{
    if (!_instance)
        _instance = new UIManager(renderer);
}

UIManager* UIManager::GetInstance()
{
    if (!_instance)
    {
        printf("UI Manager Not Init \n");
        return nullptr;
    }

    return _instance;
}

UIManager::UIManager(const Renderer& renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    ImGui_ImplSDL2_InitForOpenGL(renderer.GetDefaultWindow(), renderer.GetDefaultContext());
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void UIManager::AddUIWindow(UIWindow* window)
{
    _uiWindows.push_back(window);
}

void UIManager::RemoveUIWindow(UIWindow* window)
{
    std::erase(_uiWindows, window);
}

void UIManager::Render() const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    for (const auto window : _uiWindows)
        window->Render();

    ImGui::End();
    ImGui::Render();
}

void UIManager::CleanUp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    delete _instance;
}

bool UIManager::HoveringUI()
{
	const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
	const bool isItemHovered = ImGui::IsAnyItemHovered();
    return isWindowHovered || isItemHovered;
}

void UIManager::ClearFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Render();
    ImGui::UpdatePlatformWindows();
}
