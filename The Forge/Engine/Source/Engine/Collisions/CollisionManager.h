#pragma once
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
        bool CheckCollision(const Collider* collider, std::vector<Collider*>& returnObjects);
        
    private:
        void ResolveCollision(Rigidbody* a, Rigidbody* b, glm::vec2 normal, float penetration) const;
        void CheckCollisions(const std::vector<Collider*>& colliders);
        
    private:
        QuadTree _quadTree;
        
    };

    inline CollisionManager& GetCollisionManager()
    {
        return CollisionManager::GetInstance();
    }
}
