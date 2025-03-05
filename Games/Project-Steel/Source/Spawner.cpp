#include "Spawner.h"
#include <numbers>
#include "imgui.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/CircleCollider.h"

Spawner::Spawner(): _spawnTime(0), _timeElapsed(0), _radius(0), _initialAmount(0), _gen(_rd()), _dist(0.0f, 1.0f)
{
}

void Spawner::Start()
{
    for (int i = 0; i < _initialAmount; i++)
        SpawnObject();
}

void Spawner::Update(const float deltaTime)
{
    // Increment the elapsed time by the delta time
    _timeElapsed += deltaTime;

    // Check if it's time to spawn an object
    if (_timeElapsed >= _spawnTime) {
        SpawnObject();  // Call SpawnObject to spawn an object
        _timeElapsed = 0.0f;  // Reset the elapsed time after spawning
    }
}

void Spawner::SpawnObject()
{
    // Generate random position within the radius around the spawner
    const float angle = _dist(_gen) * 2.0f * static_cast<float>(std::numbers::pi);   // Random angle between 0 and 2π
    const float distance = _dist(_gen) * _radius;     // Random distance within the radius

    // Calculate random x and y coordinates
    const glm::vec2 spawnPos = glm::vec2(distance * std::cos(angle), distance * std::sin(angle)) + gameObject->GetWorldPosition();

    const auto go = Engine::LevelManager::GetCurrentLevel()->SpawnNewGameObject(_prefabToSpawn);
    go->SetRotation(angle * (180.f / static_cast<float>(std::numbers::pi)));
    go->SetPosition(spawnPos);
}

nlohmann::json Spawner::Serialize()
{
    nlohmann::json data;
    data["Initial Amount"] = _initialAmount;
    data["Spawn Time"] = _spawnTime;
    data["Radius"] = _radius;
    data["Prefab"] = _prefabToSpawn;
    return data;
}

void Spawner::Deserialize(const nlohmann::json& json)
{
    _initialAmount = json["Initial Amount"];
    _spawnTime = json["Spawn Time"];
    _radius = json["Radius"];
    _prefabToSpawn = json["Prefab"];
}

void Spawner::DrawDetails()
{
    ImGui::InputInt("Initial Amount", &_initialAmount);
    ImGui::Spacing();
    
    ImGui::InputFloat("Spawn Time", &_spawnTime);
    ImGui::Spacing();

    ImGui::InputFloat("Radius", &_radius);
    ImGui::Spacing();
    
    // Prefab to spawn filepath
    if (ImGui::Button("Prefab", ImVec2(50, 50))) {}
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* prefab = nullptr;
            prefab = static_cast<const char*>(payload->Data); // Retrieve the file path
            _prefabToSpawn = std::string(prefab);
        }
        ImGui::EndDragDropTarget();
    }

    // Display saved filepath for prefab
    if (!_prefabToSpawn.empty())
        ImGui::Text("Prefab Saved Path: %s", _prefabToSpawn.c_str());
}
