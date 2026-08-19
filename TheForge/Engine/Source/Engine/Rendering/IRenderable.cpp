#include "IRenderable.h"

#include "imgui.h"
#include "ImGuiHelper.h"
#include "Renderer.h"
#include "RenderingUtils.h"
#include "Editor/Console.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"

Engine::IRenderable::IRenderable(): _quadVAO(0), _resourcesResident(false), _sortingLayer(0), _isHidden(false), _gameObject(nullptr)
{
}

Engine::IRenderable::~IRenderable()
{
    // Only touch GL if something was actually created. A headless build never
    // renders, so it never has resources to release.
    if (_resourcesResident)
        shader.Reset();

    GetRenderer().RemoveComponentFromRenderList(this);
}

void Engine::IRenderable::EnsureResourcesResident()
{
    if (_resourcesResident) return;
    _resourcesResident = true;

    _quadVAO = RenderingUtils::GenerateVAO();
    shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());
}

void Engine::IRenderable::InitRenderable(GameObject* go)
{
    // The VAO is deliberately not generated here -- see EnsureResourcesResident.
    _gameObject = go;
    GetRenderer().AddComponentToRenderList(this);
}

void Engine::IRenderable::Deserialize(const nlohmann::json& data)
{
    if (data.contains(JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER))
        _vertexShaderFilepath = data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER];

    if (data.contains(JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER))
        _fragmentShaderFilepath = data[JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER];

    InvalidateResources();

    if (data.contains(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER))
        _sortingLayer = data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER];

    GetRenderer().AddComponentToRenderList(this);
}

nlohmann::json Engine::IRenderable::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = _sortingLayer;
    data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER] = _vertexShaderFilepath;
    data[JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER] = _fragmentShaderFilepath;

    return data;
}

void Engine::IRenderable::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_sortingLayer);
    stream.Write(_vertexShaderFilepath);
    stream.Write(_fragmentShaderFilepath);
}

void Engine::IRenderable::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_sortingLayer);
    stream.Read(_vertexShaderFilepath);
    stream.Read(_fragmentShaderFilepath);

    InvalidateResources();
}

void Engine::IRenderable::DrawRenderableSettings()
{
    ImGui::Checkbox("Hidden", &_isHidden);
    ImGui::Spacing();

    const std::string label = "Sorting Layer##" + std::to_string((uintptr_t)this);
    int temp = _sortingLayer;
    ImGui::InputInt(label.c_str(), &temp);
    _sortingLayer = (int8_t)temp;
    ImGui::Spacing();

    if (ImGuiHelper::DragDropFileButton("Vert", _vertexShaderFilepath, "FILE_PATH"))
    {
        InvalidateResources();
    }

    ImGui::SameLine();

    if (ImGuiHelper::DragDropFileButton("Frag", _fragmentShaderFilepath, "FILE_PATH"))
    {
        InvalidateResources();
    }

    ImGui::Spacing();

    ImGuiHelper::DisplayFilePath("Vertex Saved Path", _vertexShaderFilepath);
    ImGuiHelper::DisplayFilePath("Fragment Saved Path", _fragmentShaderFilepath);
}
