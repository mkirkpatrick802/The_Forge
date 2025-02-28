#pragma once
#include "QuadTree.h"

namespace Engine
{
    class CollisionManager
    {
    public:
        static CollisionManager& GetInstance();
        CollisionManager();
        
        void Update();

    private:
        void CheckCollisions(const std::vector<Collider*>& colliders);
        
    private:
        QuadTree _quadTree;
        
    };

    inline CollisionManager& GetCollisionManager()
    {
        return CollisionManager::GetInstance();
    }
}
