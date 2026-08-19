#include "SpaceGrid.h"

#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#include "Engine/AssetMetadata.h"
#include "Engine/EngineManager.h"
#include "Engine/GameEngine.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/ImGuiHelper.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/TextureLoader.h"

SpaceGrid::SpaceGrid() : _gen(std::random_device{}()), _gridSize(2,2)
{
    
}

SpaceGrid::~SpaceGrid()
{
    Engine::GetRenderer().RemoveComponentFromRenderList(this);
}

void SpaceGrid::OnActivation()
{
    InitRenderable(gameObject);
    if (const auto level = Engine::LevelManager::GetCurrentLevel())
        gameObject->SetPosition(level->GetLevelBottomLeft());
}

void SpaceGrid::InitTiles()
{
    _tiles.clear();
    
    // Extract weights from pairs
    std::vector<int> weights;
    for (const auto& key : _sprites | std::views::keys) {
        weights.push_back(key);
    }
    
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());

    const glm::vec2 levelSize = Engine::LevelManager::GetCurrentLevel()->GetLevelSize();
    _gridSize.x = glm::round(levelSize.x / static_cast<float>(_tileSize)) + 1;
    _gridSize.y = glm::round(levelSize.y / static_cast<float>(_tileSize)) + 1;
    
    for (int x = 0; x < static_cast<int>(_gridSize.x); x++)
    {
        for (int y = 0; y < static_cast<int>(_gridSize.y); y++)
        {
            // Select a random index based on weights
            const size_t randomIndex = dist(_gen);
            const SpriteAsset& sprite = _sprites[randomIndex].second;
            if (sprite.path.empty() || sprite.size.x <= 0.0f || sprite.size.y <= 0.0f) continue;

            glm::vec2 pos(x * (int)sprite.size.x, y * (int)sprite.size.y);
            _tiles.emplace_back(pos + gameObject->GetWorldPosition(), sprite.path);
        }
    }
}

void SpaceGrid::Render(const Engine::ShaderUniformData& data)
{
    if (IsHidden() || _tiles.empty()) return;

    for (const auto [fst, snd] : _tiles)
    {
        if (const auto camera = Engine::GetCameraManager().GetActiveCamera())
        {
            const float buffer = 400.0f;
            auto [left, top, right, bottom] = camera->GetBounds();
            glm::vec2 cameraTopRight = Engine::GetCameraManager().ConvertScreenToWorld(glm::vec2(right, top));
            glm::vec2 cameraBottomLeft = Engine::GetCameraManager().ConvertScreenToWorld(glm::vec2(left, bottom));

            // Add buffer to the bounds
            cameraTopRight += glm::vec2(buffer, buffer);
            cameraBottomLeft -= glm::vec2(buffer, buffer);

            // Check if the tile is within the buffered camera's visible area
            if (fst.x >= cameraBottomLeft.x && fst.x < cameraTopRight.x &&
                fst.y >= cameraBottomLeft.y && fst.y < cameraTopRight.y)
            {
                RenderTile(GetTileTexture(snd), fst);
            }
        }
        else if (Engine::GetEngineManager().IsEditorEnabled())
        {
            RenderTile(GetTileTexture(snd), fst);
        }
    }

}

const Engine::Texture* SpaceGrid::GetTileTexture(const std::string& path)
{
    if (path.empty()) return nullptr;

    if (const auto it = _textureCache.find(path); it != _textureCache.end())
        return it->second.get();

    auto texture = CreateTexture(path, Engine::Texture::TextureType::PIXEL);
    const Engine::Texture* raw = texture.get();
    _textureCache.emplace(path, std::move(texture));
    return raw;
}

void SpaceGrid::RenderTile(const Engine::Texture* sprite, const glm::vec2 pos)
{
    if (!sprite) return;

    shader.Use();
    auto model = glm::mat4(1.0f);
    auto screenPos = Engine::GetCameraManager().ConvertWorldToScreen(glm::vec2(pos.x - sprite->GetSize().x / 2.f, (pos.y * -1) - sprite->GetSize().y / 2.f));
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(.5f * sprite->GetSize().x, .5f * sprite->GetSize().y, 0.0f));
    model = rotate(model, 0.f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * sprite->GetSize().x, -0.5f * sprite->GetSize().y, 0.0f));

    model = scale(model, glm::vec3(sprite->GetSize(), 1.0f));
    
    glBindTextureUnit(0, sprite->GetID());

    shader.SetMatrix4("model", model);
    
    shader.SetMatrix4("projection", Engine::GetProjectionMatrix());
    shader.SetMatrix4("view", Engine::GetViewMatrix());
    shader.SetInteger("image", 0);
    
    glBindVertexArray(GetVAO());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpaceGrid::DrawDetails()
{
    DrawRenderableSettings();
    ImGui::Spacing();
    
    if (ImGui::CollapsingHeader("Sprite Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int i = 0;
        for (auto& sprite : _sprites)
        {
            ImGui::PushID(i); // Prevents ID conflicts
            
            ImGui::Separator();
            ImGui::Text("Sprite %d", i);

            // Drag & Drop File Button
            if (std::string path; Engine::ImGuiHelper::DragDropFileButton("Load", path, "FILE_PATH"))
            {
                // Importing here is what produces the sidecar a server later reads.
                Engine::EnsureImageAssetImported(path);

                sprite.second.path = path;
                sprite.second.size = Engine::GetImageSize(path);
                _textureCache.erase(path);
            }

            if (!sprite.second.path.empty())
                Engine::ImGuiHelper::DisplayFilePath("Path", sprite.second.path);

            ImGui::SameLine();
            
            // Weight Input
            ImGui::PushItemWidth(50);
            ImGui::InputInt("Spawn Weight", &sprite.first, 0);
            ImGui::PopItemWidth();

            ImGui::PopID();
            i++;
        }
    }

    // Button to add new sprite entry
    ImGui::Spacing();
    if (ImGui::Button("Add New Sprite"))
    {
        _sprites.emplace_back(1, nullptr);
    }
}

void SpaceGrid::Deserialize(const json& data)
{
    IRenderable::Deserialize(data);
    if (data.contains("grid size x") && data.contains("grid size y"))
    {
        _gridSize.x = static_cast<float>(data["grid size x"]);
        _gridSize.y = static_cast<float>(data["grid size y"]);
    }

    // Deserialize _sprites
    if (data.contains("sprites"))
    {
        const auto& sprites_json = data["sprites"];
        
        // Clear existing sprites before deserializing
        _sprites.clear();

        // Loop through each sprite in the JSON array
        for (const auto& sprite_data : sprites_json)
        {
            int sprite_id = sprite_data["weight"];

            SpriteAsset sprite;
            if (sprite_data.contains("texture"))
            {
                sprite.path = sprite_data["texture"];
                sprite.size = Engine::GetImageSize(sprite.path);
            }

            // Add the deserialized sprite to _sprites
            _sprites.emplace_back(sprite_id, std::move(sprite));
        }
    }

    InitTiles();
}

nlohmann::json SpaceGrid::Serialize()
{
    nlohmann::json data = IRenderable::Serialize();
    data["grid size x"] = _gridSize.x;
    data["grid size y"] = _gridSize.y;

    // Serialize _sprites
    nlohmann::json sprites_json = nlohmann::json::array();
    for (const auto& sprite : _sprites)
    {
        nlohmann::json sprite_data;
        sprite_data["weight"] = sprite.first;

        if (!sprite.second.path.empty())
            sprite_data["texture"] = sprite.second.path;

        sprites_json.push_back(sprite_data);
    }
    
    data["sprites"] = sprites_json;
    
    return data;
}

void SpaceGrid::Write(NetCode::OutputByteStream& stream) const
{
    IRenderable::Write(stream);
    stream.Write(_gridSize);

    const uint32_t size = static_cast<uint32_t>(_tiles.size());
    stream.Write(size);
    for (const auto& [fst, snd] : _tiles)
    {
        stream.Write(fst);
        stream.Write(snd);
    }
}

void SpaceGrid::Read(NetCode::InputByteStream& stream)
{
    IRenderable::Read(stream);
    stream.Read(_gridSize);

    uint32_t size;
    stream.Read(size);
    for (uint32_t i = 0; i < size; i++)
    {
        glm::vec2 pos;
        std::string filepath;
        stream.Read(pos);
        stream.Read(filepath);
        _tiles.emplace_back(pos, filepath);
    }

    Engine::GetRenderer().AddComponentToRenderList(this);
}
