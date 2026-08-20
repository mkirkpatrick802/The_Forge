#include "ScenePicker.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Engine/GameObject.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/SpriteRenderer.h"

int Editor::ScenePicker::_lastPickIndex = -1;

namespace
{
    bool ContainsPoint(const Engine::SpriteRenderer& sprite, const glm::vec2 world)
    {
        const glm::vec2 size = sprite.GetSize();
        if (size.x <= 0.0f || size.y <= 0.0f) return false;

        const Engine::GameObject* go = sprite.gameObject;
        if (go == nullptr) return false;

        // The sprite quad is rotated about its centre (see SpriteRenderer::CollectUniforms),
        // so the point is rotated back into the object's own frame rather than the bounds
        // being grown to enclose the rotated quad -- otherwise a rotated sprite would be
        // selectable well outside itself.
        const float radians = glm::radians(-go->GetWorldRotation());
        const float cosR = std::cos(radians);
        const float sinR = std::sin(radians);

        const glm::vec2 offset = world - go->GetWorldPosition();
        const glm::vec2 local(offset.x * cosR - offset.y * sinR,
                              offset.x * sinR + offset.y * cosR);

        return std::abs(local.x) <= size.x * 0.5f
            && std::abs(local.y) <= size.y * 0.5f;
    }
}

Engine::GameObject* Editor::ScenePicker::Pick(const glm::vec2 worldPosition, const bool advance)
{
    const auto pool = Engine::GetComponentManager().GetPool<Engine::SpriteRenderer>();
    if (pool == nullptr)
    {
        _lastPickIndex = -1;
        return nullptr;
    }

    std::vector<Engine::SpriteRenderer*> hits;
    for (Engine::SpriteRenderer* sprite : pool->GetActive())
    {
        if (sprite == nullptr || sprite->gameObject == nullptr) continue;

        // Hidden sprites are not on screen to be clicked, and screen-space ones are not
        // in the world at all -- they bypass the view matrix entirely, so a world
        // position means nothing to them.
        if (sprite->IsHidden() || sprite->IsScreenSpace()) continue;

        if (ContainsPoint(*sprite, worldPosition))
            hits.push_back(sprite);
    }

    if (hits.empty())
    {
        _lastPickIndex = -1;
        return nullptr;
    }

    // Front-most first. The renderer draws in ascending sorting layer, so the highest
    // layer is what the click visually landed on.
    std::ranges::stable_sort(hits, [](const Engine::SpriteRenderer* a, const Engine::SpriteRenderer* b)
    {
        return a->GetSortingLayer() > b->GetSortingLayer();
    });

    if (advance && _lastPickIndex >= 0)
        _lastPickIndex = (_lastPickIndex + 1) % static_cast<int>(hits.size());
    else
        _lastPickIndex = 0;

    return hits[_lastPickIndex]->gameObject;
}
