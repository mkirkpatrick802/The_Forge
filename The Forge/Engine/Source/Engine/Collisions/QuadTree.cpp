#include "QuadTree.h"

#include "Engine/GameEngine.h"
#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/Renderer.h"
#include <glm/glm.hpp>

#include "Engine/Rendering/DebugRenderer.h"

Engine::QuadTree::QuadTree(): _depth(0), _position(), _size(), _hasChildren(false)
{
    
}

Engine::QuadTree::QuadTree(const int depth, const glm::vec2 position, const glm::vec2 size):
_depth(depth), _position(position), _size(size), _hasChildren(false)
{
    
}

void Engine::QuadTree::DebugRender()
{
    // Define the color for the QuadTree boundaries
    glm::vec3 color(1.0f, 0.0f, 0.0f); // Red for boundaries

    // Render the boundary of this QuadTree node using DebugRenderer
    GetDebugRenderer().DrawRectangle(_position + glm::vec2(_size.x * 0.5f, _size.y * -.5f), _size, color);

    // Define colors for colliders
    glm::vec3 circleColor(0.0f, 1.0f, 0.0f);   // Green for circles
    glm::vec3 rectColor(0.0f, 0.0f, 1.0f);     // Blue for rectangles
    
    for (const auto collider : _objects)
    {
        if (!collider->gameObject) continue;

        if (collider->GetColliderType() == EColliderType::ECT_Circle)
        {
            const auto circle = dynamic_cast<CircleCollider*>(collider);
            glm::vec2 center = circle->gameObject->GetWorldPosition();
            const float radius = circle->GetRadius();

            // Draw circle using DebugRenderer
            GetDebugRenderer().DrawCircle(center, radius, circleColor);
        }

        if (collider->GetColliderType() == EColliderType::ECT_Rectangle)
        {
            const auto rect = dynamic_cast<RectangleCollider*>(collider);
            glm::vec2 size = rect->GetSize();
            glm::vec2 center = rect->gameObject->GetWorldPosition();

            // Draw rectangle using DebugRenderer
            GetDebugRenderer().DrawRectangle(center, size, rectColor);
        }
    }

    // If the QuadTree has children, recursively render them
    if (_hasChildren)
    {
        for (const auto& child : _children)
        {
            if (child)
                child->DebugRender();
        }
    }
}


void Engine::QuadTree::Insert(Collider* collider)
{
    // Ensure the collider is within the bounds of the QuadTree before proceeding
    const glm::vec2 colliderPos = collider->gameObject->GetWorldPosition();
    
    // Check if the colliders position is inside the QuadTree's bounds
    const auto [min, max] = GetColliderMinMax(collider);
    const bool isInBounds = max.x >= _position.x && min.x <= (_position.x + _size.x) &&
                            min.y <= _position.y && max.y >= (_position.y - _size.y);


    
    if (!isInBounds) return;

    _objects.push_back(collider);

    if (_objects.size() > MAX_QUADTREE_OBJECTS && _depth < MAX_QUADTREE_DEPTH)
    {
        if (!_hasChildren)
            Split();

        auto it = _objects.begin();
        while (it != _objects.end())
        {
            auto indices = GetIndices(*it);
            if (indices.empty()) 
            {
                ++it;
                continue;
            }

            // Insert into all overlapping children
            for (const auto index : indices)
            {
                _children[index]->Insert(*it);
            }

            // Erase safely and update iterator
            it = _objects.erase(it);
        }
    }

}

void Engine::QuadTree::Retrieve(std::vector<Collider*>& returnObjects, const Collider* collider)
{
    const auto indices = GetIndices(collider);
    if (_hasChildren)
    {
        for (const auto index : indices)
            _children[index]->Retrieve(returnObjects, collider);
    }

    returnObjects.insert(returnObjects.end(), _objects.begin(), _objects.end());
}

void Engine::QuadTree::Retrieve(std::vector<Collider*>& returnObjects, const glm::vec2 point)
{
    if (_hasChildren)
       if (const int index = GetIndex(point); index != -1)
            _children[index]->Retrieve(returnObjects, point);

    returnObjects.insert(returnObjects.end(), _objects.begin(), _objects.end());
}

void Engine::QuadTree::Split()
{
    glm::vec2 halfSize = _size * 0.5f;

    // Calculate child positions based in the center of the parent QuadTree
    glm::vec2 center = _position + glm::vec2(halfSize.x, -halfSize.y); // The center of the current QuadTree

    _children[0] = std::make_unique<QuadTree>(_depth + 1, glm::vec2(_position.x, _position.y), halfSize); // Top-left
    _children[1] = std::make_unique<QuadTree>(_depth + 1, glm::vec2(center.x, _position.y), halfSize); // Top-right
    _children[2] = std::make_unique<QuadTree>(_depth + 1, glm::vec2(_position.x, center.y), halfSize); // Bottom-left
    _children[3] = std::make_unique<QuadTree>(_depth + 1, glm::vec2(center.x, center.y), halfSize); // Bottom-right

    _hasChildren = true;
}


std::vector<int> Engine::QuadTree::GetIndices(const Collider* collider) const
{
    const auto [min, max] = GetColliderMinMax(collider);
    return GetIndices(min, max);
}

std::vector<int> Engine::QuadTree::GetIndices(const glm::vec2 min, const glm::vec2 max) const
{
    std::vector<int> indices;

    // Quadrant boundaries
    const float midX = _position.x + _size.x * .5f;
    const float midY = _position.y - _size.y * .5f;

    const bool left = min.x < midX;
    const bool right = max.x > midX;
    const bool top = max.y > midY;
    const bool bottom = min.y < midY;

    // Check overlapping quadrants
    if (top && left) indices.push_back(0);
    if (top && right) indices.push_back(1);
    if (bottom && left) indices.push_back(2);
    if (bottom && right) indices.push_back(3);

    return indices;
}

int Engine::QuadTree::GetIndex(const glm::vec2 point) const
{
    const bool top = point.y > _position.y - _size.y * 0.5f;
    const bool left = point.x < _position.x + _size.x * 0.5f;

    if (top)
        return left ? 0 : 1;

    return left ? 2 : 3;
}

std::pair<glm::vec2, glm::vec2> Engine::QuadTree::GetColliderMinMax(const Collider* collider) const
{
    if (collider->GetColliderType() == EColliderType::ECT_Rectangle)
    {
        const auto* rect = dynamic_cast<const RectangleCollider*>(collider);
        const glm::vec2 halfSize = rect->GetSize() * 0.5f;
        const glm::vec2 rectMin = rect->gameObject->GetWorldPosition() - halfSize;
        const glm::vec2 rectMax = rect->gameObject->GetWorldPosition() + halfSize;

        return {rectMin, rectMax};
    }
    
    if (collider->GetColliderType() == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(collider);
        const glm::vec2 center = circle->gameObject->GetWorldPosition();
        const float radius = circle->GetRadius();

        return {center - glm::vec2(radius), center + glm::vec2(radius)};
    }
    
    return std::make_pair(glm::vec2(), glm::vec2());
}

void Engine::QuadTree::Clear()
{
    _objects.clear();
    if (_hasChildren)
    {
        for (auto& i : _children)
            i.reset();
        
        _hasChildren = false;
    }
}
