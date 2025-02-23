#include "SpriteRenderer.h"

#include "Engine/GameObject.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ComponentRegistry.h"
#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::SpriteRenderer::SpriteRenderer(): _quadVAO(0), _size(), _hidden(false)
{
}

void Engine::SpriteRenderer::OnActivation()
{
    // configure VAO/VBO
    unsigned int VBO;
    const float vertices[] = {
        // pos         // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &_quadVAO);
    glBindVertexArray(_quadVAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GetRenderer().AddComponentToRenderList(this);
}

void Engine::SpriteRenderer::CollectUniforms(ShaderUniformData& data)
{
    if (!_texture || _hidden) return;
    const float rotation = gameObject->transform.rotation;
    const auto position = glm::vec2(gameObject->transform.position.x - _size.x / 2, (gameObject->transform.position.y * -1) - _size.y / 2);
    
    auto model = glm::mat4(1.0f);
    auto screenPos = GetCameraManager().ConvertWorldToScreen(position);
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.0f));
    model = rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    model = scale(model, glm::vec3(_size, 1.0f));

    glBindTextureUnit(0, _texture->GetID());
    
    data.intUniforms["image"] = 0;
    data.mat4Uniforms["model"] = model;
    data.mat4Uniforms["projection"] = GetProjectionMatrix();
    data.mat4Uniforms["view"] = GetViewMatrix();
}

void Engine::SpriteRenderer::Render(const ShaderUniformData& data)
{
    _shader.Use();
    
    for (const auto& [name, value] : data.intUniforms)
        _shader.SetInteger(name.c_str(), value);
    
    for (const auto& [name, value] : data.floatUniforms)
        _shader.SetFloat(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec2Uniforms)
        _shader.SetVector2f(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec3Uniforms)
        _shader.SetVector3f(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec4Uniforms)
        _shader.SetVector4f(name.c_str(), value);

    for (const auto& [name, value] : data.mat4Uniforms)
        _shader.SetMatrix4(name.c_str(), value);
    
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// Serialization 
void Engine::SpriteRenderer::Deserialize(const json& data)
{
    const std::string filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
    _size = _texture->GetSize();

    if (data.contains(JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER))
        _vertexShaderFilepath = data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER];

    if (data.contains(JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER))
        _fragmentShaderFilepath = data[JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER];

    _shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());
    if (data.contains(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER))
        sortingLayer = data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER];
}

nlohmann::json Engine::SpriteRenderer::Serialize()
{
    nlohmann::json data;

    if (_texture)
    {
        std::string filepath = _texture->GetFilePath();
        data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    }
    
    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = sortingLayer;
    data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER] = _vertexShaderFilepath;
    data[JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER] = _fragmentShaderFilepath;
    return data;
}

void Engine::SpriteRenderer::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(sortingLayer);
    stream.Write(_texture->GetFilePath());
    stream.Write(_vertexShaderFilepath);
    stream.Write(_fragmentShaderFilepath);
}

void Engine::SpriteRenderer::Read(NetCode::InputByteStream& stream)
{
    stream.Read(sortingLayer);
    
    std::string filepath;
    stream.Read(filepath);

    if (!_texture)
    {
        _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
        _size = _texture->GetSize();
    }

    stream.Read(_vertexShaderFilepath);
    stream.Read(_fragmentShaderFilepath);
    
    _shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());
}

Engine::SpriteRenderer::~SpriteRenderer()
{
    _texture.reset();
    _shader.Reset();
    GetRenderer().RemoveComponentFromRenderList(this);
}

// Find a better place to put editor code
void Engine::SpriteRenderer::DrawDetails()
{
    ImGui::Checkbox("Hidden", &_hidden);
    ImGui::Spacing();

    std::string label = "Sorting Layer##" + std::to_string((uintptr_t)this);
    ImGui::InputInt(label.c_str(), &sortingLayer);

    // Sprite Filepath
    if (ImGui::Button("Sprite", ImVec2(50, 50))) {}
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* sprite = nullptr;
            sprite = static_cast<const char*>(payload->Data); // Retrieve the file path
            
            _texture.reset();
            _texture = CreateTexture(sprite, Texture::TextureType::PIXEL);
        }
        ImGui::EndDragDropTarget();
    }

    // Display saved filepath for sprites
    if (_texture)
        if (!_texture->GetFilePath().empty())
            ImGui::Text("Sprite Saved Path: %s", _texture->GetFilePath().c_str());

    ImGui::Spacing();

    // Vertex
    if (ImGui::Button("Vert", ImVec2(50, 50))) {}
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* vert = nullptr;
            vert = static_cast<const char*>(payload->Data);
            _vertexShaderFilepath = vert;
            _shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    
    // Fragment
    if (ImGui::Button("Frag", ImVec2(50, 50))) {}
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* frag = nullptr;
            frag = static_cast<const char*>(payload->Data);
            _fragmentShaderFilepath = frag;
            _shader.Compile(_vertexShaderFilepath.c_str(), _fragmentShaderFilepath.c_str());
        }
        ImGui::EndDragDropTarget();
    }
    
    // Display saved filepath for vertex shader
    ImGui::Text("Vertex Saved Path: %s", _vertexShaderFilepath.c_str());

    // Display saved filepath for fragment shader
    ImGui::Text("Fragment Saved Path: %s", _fragmentShaderFilepath.c_str());
}
