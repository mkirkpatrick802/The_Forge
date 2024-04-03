//
// Created by mKirkpatrick on 1/22/2024.
//

#include "UIManager.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include "SDL_opengl.h"

UIManager::UIManager(Renderer& renderer) {

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    _renderer = renderer;

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(renderer.GetDefaultWindow(), renderer.GetDefaultContext());
    ImGui_ImplOpenGL3_Init(glsl_version);

}

void UIManager::Render(EditorSettings &editorSettings, GameObjectSettings *selectedGameObjectSettings) {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Menu");

    if (ImGui::CollapsingHeader("Editor Settings")) {
        if(ImGui::Checkbox("Edit Mode", &editorSettings.editMode))
            editorSettings.editModeChanged = true;
    }

    if (ImGui::CollapsingHeader("Player Settings")) {

    }

    if(selectedGameObjectSettings != nullptr) {

        ImGui::Text(" ");
        ImGui::Text("%s", selectedGameObjectSettings->name->c_str());

        if (ImGui::CollapsingHeader("Game Object Settings")) {
            std::string currentName = *selectedGameObjectSettings->name;
            char buf[256];
            strncpy_s(buf, currentName.c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = 0;

            if (ImGui::InputText("Name", buf, sizeof(buf))) {
                currentName = std::string(buf);
                *selectedGameObjectSettings->name = currentName;
            }

            float position[2] = {selectedGameObjectSettings->position->x, selectedGameObjectSettings->position->y};
            ImGui::InputFloat2("Position", position);
            *selectedGameObjectSettings->position = Vector2D(position[0], position[1]);
        }
    }

    ImGui::End();
    ImGui::Render();
}

void UIManager::CleanUp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

bool UIManager::HoveringUI()
{
	const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
	const bool isItemHovered = ImGui::IsAnyItemHovered();
    return isWindowHovered || isItemHovered;
}

void UIManager::ClearFrame()
{
    ImGui::NewFrame();
    ImGui::Render();
}
