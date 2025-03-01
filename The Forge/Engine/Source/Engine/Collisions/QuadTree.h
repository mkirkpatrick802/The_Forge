#pragma once
#include <glm/vec2.hpp>

#include "Collider.h"

namespace Engine
{
    constexpr int MAX_QUADTREE_OBJECTS = 4;
    constexpr int MAX_QUADTREE_DEPTH = 5;
    constexpr int QUADTREE_CHILDREN_COUNT = 4;
    
    class QuadTree
    {
    public:
        QuadTree(int depth, glm::vec2 position, glm::vec2 size);

        void DebugRender();
        
        void Clear();
        void Insert(Collider* collider);
        void Retrieve(std::vector<Collider*>& returnObjects, const Collider* collider);
        void Retrieve(std::vector<Collider*>& returnObjects, glm::vec2 point);

    private:
        void Split();
        int GetIndex(const Collider* collider) const;
        int GetIndex(glm::vec2 point) const;

    private:
        int _depth;                                                         // Depth level of this node
        glm::vec2 _position;                                                // Top-left position of this node
        glm::vec2 _size;                                                    // Width and height of this node
        std::vector<Collider*> _objects;                                    // Colliders in this node
        std::unique_ptr<QuadTree> _children[QUADTREE_CHILDREN_COUNT];       // Child nodes
        bool _hasChildren;                                                  // Tracks if this node is subdivided
    };
}
