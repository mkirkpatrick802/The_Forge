#include "QuadTree.h"

#include "Engine/GameEngine.h"
#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/Renderer.h"

Engine::QuadTree::QuadTree(const int depth, const glm::vec2 position, const glm::vec2 size):
_depth(depth), _position(position), _size(size), _hasChildren(false) { }

void Engine::QuadTree::DebugRender()
{
    // Define the color for the QuadTree boundaries
    glm::vec3 color(1.0f, 0.0f, 0.0f); // Red for boundaries

    // Render the boundary of this QuadTree node
    glm::vec2 topLeft = _position;
    glm::vec2 bottomRight = _position + glm::vec2(_size.x, -_size.y);

    // Draw the lines of the QuadTree (rectangle)
    GetRenderer().RenderLine(topLeft, glm::vec2(bottomRight.x, topLeft.y), color); // Top side
    GetRenderer().RenderLine(glm::vec2(bottomRight.x, topLeft.y), bottomRight, color); // Right side
    GetRenderer().RenderLine(bottomRight, glm::vec2(topLeft.x, bottomRight.y), color); // Bottom side
    GetRenderer().RenderLine(glm::vec2(topLeft.x, bottomRight.y), topLeft, color); // Left side

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
    glm::vec2 colliderPos = collider->gameObject->transform.position;
    
    // Check if the colliders position is inside the QuadTree's bounds
    bool isInBounds = colliderPos.x >= _position.x && colliderPos.x <= (_position.x + _size.x) &&
                      colliderPos.y <= _position.y && colliderPos.y >= (_position.y - _size.y);
    
    if (!isInBounds) return;
    
    if (_hasChildren)
    {
        if (const int index = GetIndex(collider); index != -1)
        {
            _children[index]->Insert(collider);
            return;
        }
    }

    _objects.push_back(collider);

    if (_objects.size() > MAX_QUADTREE_OBJECTS && _depth < MAX_QUADTREE_DEPTH)
    {
        if (!_hasChildren)
            Split();

        auto it = _objects.begin();
        while (it != _objects.end())
        {
            if (const int index = GetIndex(*it); index != -1)
            {
                _children[index]->Insert(*it);
                it = _objects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void Engine::QuadTree::Retrieve(std::vector<Collider*>& returnObjects, const Collider* collider)
{
    if (_hasChildren)
        if (const int index = GetIndex(collider); index != -1)
            _children[index]->Retrieve(returnObjects, collider);

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


int Engine::QuadTree::GetIndex(const Collider* collider) const
{
    if (collider->GetType() == EColliderType::ECT_Rectangle)
    {
        const auto* rect = dynamic_cast<const RectangleCollider*>(collider);
        const glm::vec2 rectCenter = rect->gameObject->transform.position + rect->GetSize() * 0.5f;
        return GetIndex(rectCenter);
    }
    
    if (collider->GetType() == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(collider);
        return GetIndex(circle->gameObject->transform.position);
    }

    return -1;
}

int Engine::QuadTree::GetIndex(const glm::vec2 point) const
{
    const bool top = point.y > _position.y - _size.y * 0.5f;
    const bool left = point.x < _position.x + _size.x * 0.5f;

    if (top)
        return left ? 0 : 1;

    return left ? 2 : 3;
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
