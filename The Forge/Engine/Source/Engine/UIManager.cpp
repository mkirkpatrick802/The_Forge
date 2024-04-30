#include "UIManager.h"

#include "Renderer.h"
#include "System.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

std::vector<Engine::UIWindow*> Engine::UIManager::_uiWindows = std::vector<UIWindow*>();

void Engine::UIManager::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = false;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(System::GetWindow(), Renderer::GetContext());
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Engine::UIManager::RenderWindows()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(!_uiWindows.empty())
    {
        for (const auto window : _uiWindows)
            window->Render();
    }
    else
    {
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::UIManager::FinishUIRender()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::GetDrawData())
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
