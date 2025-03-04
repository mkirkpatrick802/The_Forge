#include "QuadTree.h"

#include "Engine/GameEngine.h"
#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/Renderer.h"
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

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

    // Define colors for colliders
    glm::vec3 circleColor(0.0f, 1.0f, 0.0f);   // Green for circles
    glm::vec3 rectColor(0.0f, 0.0f, 1.0f);     // Blue for rectangles
    
    for (const auto collider : _objects)
    {
        if (!collider->gameObject) continue;
        if (collider->GetType() == EColliderType::ECT_Circle)
        {
            auto circle = dynamic_cast<CircleCollider*>(collider);
            glm::vec2 center = circle->gameObject->transform.position;
            float radius = circle->GetRadius();

            // Draw circle by approximating it with line segments
            int segments = 16;  // Number of line segments to approximate the circle
            for (int i = 0; i < segments; ++i)
            {
                float angle1 = (i * 2 * glm::pi<float>()) / segments;
                float angle2 = ((i + 1) * 2 * glm::pi<float>()) / segments;

                glm::vec2 point1 = center + glm::vec2(cos(angle1) * radius, sin(angle1) * radius);
                glm::vec2 point2 = center + glm::vec2(cos(angle2) * radius, sin(angle2) * radius);

                GetRenderer().RenderLine(point1, point2, circleColor); // Draw each segment
            }
        }

        if (collider->GetType() == EColliderType::ECT_Rectangle)
        {
            auto rect = dynamic_cast<RectangleCollider*>(collider);
            glm::vec2 halfSize = rect->GetSize() * 0.5f;
            glm::vec2 rectTopLeft = rect->gameObject->transform.position - glm::vec2(halfSize.x, -halfSize.y); 
            glm::vec2 rectBottomRight = rect->gameObject->transform.position + glm::vec2(halfSize.x, -halfSize.y); 

            // Draw rectangle collider
            GetRenderer().RenderLine(rectTopLeft, glm::vec2(rectBottomRight.x, rectTopLeft.y), rectColor); // Top
            GetRenderer().RenderLine(glm::vec2(rectBottomRight.x, rectTopLeft.y), rectBottomRight, rectColor); // Right
            GetRenderer().RenderLine(rectBottomRight, glm::vec2(rectTopLeft.x, rectBottomRight.y), rectColor); // Bottom
            GetRenderer().RenderLine(glm::vec2(rectTopLeft.x, rectBottomRight.y), rectTopLeft, rectColor); // Left
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
    glm::vec2 colliderPos = collider->gameObject->transform.position;
    
    // Check if the colliders position is inside the QuadTree's bounds
    bool isInBounds = colliderPos.x >= _position.x && colliderPos.x <= (_position.x + _size.x) &&
                      colliderPos.y <= _position.y && colliderPos.y >= (_position.y - _size.y);
    
    if (!isInBounds) return;
    
    if (_hasChildren)
    {
        for (const auto index : GetIndices(collider))
        {
            _children[index]->Insert(collider);
        }

        if (!GetIndices(collider).empty())
            return;
    }

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
    if (_hasChildren)
        for (const auto index : GetIndices(collider))
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


std::vector<int> Engine::QuadTree::GetIndices(const Collider* collider) const
{
    std::vector<int> indices;

    if (collider->GetType() == EColliderType::ECT_Rectangle)
    {
        const auto* rect = dynamic_cast<const RectangleCollider*>(collider);
        const glm::vec2 halfSize = rect->GetSize() * 0.5f;
        const glm::vec2 rectMin = rect->gameObject->transform.position - halfSize;
        const glm::vec2 rectMax = rect->gameObject->transform.position + halfSize;

        return GetIndices(rectMin, rectMax);
    }
    
    if (collider->GetType() == EColliderType::ECT_Circle)
    {
        const auto* circle = dynamic_cast<const CircleCollider*>(collider);
        const glm::vec2 center = circle->gameObject->transform.position;
        const float radius = circle->GetRadius();

        return GetIndices(center - glm::vec2(radius), center + glm::vec2(radius));
    }

    return indices;
}

std::vector<int> Engine::QuadTree::GetIndices(const glm::vec2 min, const glm::vec2 max) const
{
    std::vector<int> indices;

    // Quadrant boundaries
    const float midX = _position.x + _size.x * 0.5f;
    const float midY = _position.y - _size.y * 0.5f;

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
