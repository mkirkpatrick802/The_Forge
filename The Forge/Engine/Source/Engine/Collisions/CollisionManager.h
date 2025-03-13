#pragma once
#include <unordered_set>

#include "QuadTree.h"

namespace Engine
{
    class Rigidbody;

    class CollisionManager
    {
        friend class GameEngine;
        
    public:
        static CollisionManager& GetInstance();
        CollisionManager();
        void Update();

        bool CheckCollisions(glm::vec2 point, std::vector<Collider*>& returnObjects);
        bool CheckCollisions(const Collider* collider, std::vector<Collider*>& returnObjects);
        
    private:
        void ResolveCollision(Rigidbody* a, Rigidbody* b, glm::vec2 normal, float penetration) const;
        void CheckCollisions(const std::vector<Collider*>& colliders);
        
    private:
        QuadTree _quadTree;

        // TODO: use game objects instead of colliders
        std::unordered_map<const Collider*, std::unordered_set<const Collider*>> _activeOverlaps;
    };

    inline CollisionManager& GetCollisionManager()
    {
        return CollisionManager::GetInstance();
    }
}
