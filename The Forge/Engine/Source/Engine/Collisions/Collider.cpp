#include "Collider.h"

#include <iostream>

void Engine::Collider::OnActivation()
{
    Component::OnActivation();
}

bool Engine::Collider::CheckCollision(const Collider* collider) const
{
    std::cerr << "Check collision function not overridden!!!" << '\n';
    return false;
}

bool Engine::Collider::CheckCollision(glm::vec2 pos) const
{
    std::cerr << "Check collision function not overridden!!!" << '\n';
    return false;
}
