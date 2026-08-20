#include "IRenderable.h"

#include "imgui.h"
#include "ImGuiHelper.h"
#include "Renderer.h"
#include "RenderingUtils.h"
#include "Editor/Console.h"
#include "Engine/GameEngine.h"
#include "Engine/GameObject.h"
#include "Engine/JsonKeywords.h"
#include "Engine/System.h"

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

    // A compile that failed is retried, but only once the paths differ from the ones
    // that failed. Marking resources resident before knowing whether the shader built
    // is what used to make a bad compile permanent for the life of the process.
    if (_shaderCompileFailed
        && _vertexShaderFilepath == _compiledVertexShader
        && _fragmentShaderFilepath == _compiledFragmentShader)
        return;

    if (_quadVAO == 0)
        _quadVAO = RenderingUtils::GenerateVAO();

    _compiledVertexShader = _vertexShaderFilepath;
    _compiledFragmentShader = _fragmentShaderFilepath;

    shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());

    _shaderCompileFailed = !shader.IsValid();
    if (_shaderCompileFailed)
    {
        // Named, because "a shader failed" on its own leaves you grepping the scene for
        // which object it belongs to.
        DEBUG_LOG("Shader failed to build on '%s': '%s' + '%s'",
            _gameObject != nullptr ? _gameObject->GetName().c_str() : "(no object)",
            _vertexShaderFilepath.empty() ? "(none)" : _vertexShaderFilepath.c_str(),
            _fragmentShaderFilepath.empty() ? "(none)" : _fragmentShaderFilepath.c_str())
        return;
    }

    _resourcesResident = true;
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

    if (const auto layer = static_cast<int8_t>(temp); layer != _sortingLayer)
    {
        _sortingLayer = layer;

        // The render list is keyed by sorting layer, so changing it here has no effect
        // until the renderable is re-inserted. Without this the new value showed in the
        // inspector but drew in the old order until the level was reloaded -- which is
        // why hitting Play appeared to "fix" it.
        GetRenderer().AddComponentToRenderList(this);
    }

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
