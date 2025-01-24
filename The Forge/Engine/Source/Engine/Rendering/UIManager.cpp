#include "UIManager.h"

#include "Renderer.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include <utility>

#include "Engine/EngineManager.h"
#include "Engine/System.h"

std::vector<std::pair<bool, std::shared_ptr<Engine::UIWindow>>> Engine::UIManager::_uiWindows;
std::vector<std::shared_ptr<Engine::UIWindow>> Engine::UIManager::_windowsToAdd;
bool Engine::UIManager::_isDockingEnabled = false; 

void Engine::UIManager::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigDockingWithShift = false; // This makes docking require holding shift

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(System::GetWindow(), Renderer::GetContext());
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Engine::UIManager::AddUIWindow(const std::shared_ptr<UIWindow>& window)
{
    _windowsToAdd.push_back(window);
}

void Engine::UIManager::RemoveUIWindow(const std::shared_ptr<UIWindow>& window)
{
    // Use std::ranges::find_if to locate the pair
    const auto it = std::ranges::find_if(_uiWindows, [&window](const std::pair<bool, std::shared_ptr<UIWindow>>& p) {
        return p.second == window;
    });

    // If the pair is found, set the bool (first) to true
    if (it != _uiWindows.end())
    {
        it->first = true;
    }
}

void Engine::UIManager::RemoveAllUIWindows()
{
    for (auto& key : _uiWindows | std::views::keys)
    {
        key = true;
    }
}

void Engine::UIManager::RenderWindows()
{
    CheckForUpdates();
    if (_uiWindows.empty()) return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (_isDockingEnabled)
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    }
        
    for (const auto& val : _uiWindows | std::views::values)
        val->Render();

    ImGui::Render();
    FinishUIRender();
}

void Engine::UIManager::CheckForUpdates()
{
    // Remove windows that are marked for removal 
    for (auto it = _uiWindows.begin(); it != _uiWindows.end(); )
    {
        if (it->first)
        {
            it = _uiWindows.erase(it);
            continue;
        }
        
        ++it;
    }

    // Add windows marked to be added
    for (auto window : _windowsToAdd)
    {
        auto pair = std::make_pair(false, window);
        _uiWindows.push_back(pair);
    }

    _windowsToAdd.clear();
    _windowsToAdd.shrink_to_fit();
}

void Engine::UIManager::FinishUIRender()
{
    if (ImGui::GetDrawData())
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        const ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            const SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
    }
}

void Engine::UIManager::CleanUp()
{
    _uiWindows.clear();
    _uiWindows.shrink_to_fit();

    _windowsToAdd.clear();
    _windowsToAdd.shrink_to_fit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}