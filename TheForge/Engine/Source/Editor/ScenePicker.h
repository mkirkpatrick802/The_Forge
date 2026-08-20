#pragma once
#include <glm/glm.hpp>

namespace Engine
{
    class GameObject;
}

namespace Editor
{
    // Finds the game object under a point in the editor viewport.
    //
    // Hit-tests against **sprite bounds** rather than colliders, for two reasons: an
    // object you can see should be selectable whether or not it has a collider, and
    // CollisionManager's quadtree is only populated by its Update, which does not run
    // in editor mode -- a point query there would always come back empty.
    class ScenePicker
    {
    public:
        // Returns the front-most object at `worldPosition`, or nullptr if there is
        // nothing there.
        //
        // `advance` asks for the *next* candidate rather than the front-most one, which
        // is how clicking the same spot repeatedly steps down through a stack. The
        // caller decides what "the same spot" means, because it is a question about
        // pixels and this function works in world units.
        static Engine::GameObject* Pick(glm::vec2 worldPosition, bool advance);

    private:
        static int _lastPickIndex;
    };
}
