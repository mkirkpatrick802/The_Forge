#pragma once
#include <memory>
#include <vector>
#include <glm/vec2.hpp>

namespace Engine
{
    class Collider;
    constexpr int MAX_QUADTREE_OBJECTS = 10;
    constexpr int MAX_QUADTREE_DEPTH = 8;
    constexpr int QUADTREE_CHILDREN_COUNT = 4;
    
    class QuadTree
    {
    public:
        QuadTree();
        QuadTree(int depth, glm::vec2 position, glm::vec2 size);

        void DebugRender();
        
        void Clear();
        void Insert(Collider* collider);
        void Retrieve(std::vector<Collider*>& returnObjects, const Collider* collider);
        void Retrieve(std::vector<Collider*>& returnObjects, glm::vec2 point);

    private:
        void Split();
        std::vector<int> GetIndices(const Collider* collider) const;
        std::vector<int> GetIndices(glm::vec2 min, glm::vec2 max) const;
        int GetIndex(glm::vec2 point) const;

        std::pair<glm::vec2, glm::vec2> GetColliderMinMax(const Collider* collider) const;

    private:
        int _depth;                                                         // Depth level of this node
        glm::vec2 _position;                                                // Top-left position of this node
        glm::vec2 _size;                                                    // Width and height of this node
        std::vector<Collider*> _objects;                                    // Colliders in this node
        std::unique_ptr<QuadTree> _children[QUADTREE_CHILDREN_COUNT];       // Child nodes
        bool _hasChildren;                                                  // Tracks if this node is subdivided
    };
}
