#include "Rigidbody.h"

Engine::Rigidbody::Rigidbody(): _velocity(), _acceleration(), _mass(1), _inverseMass(0), _static(false)
{
}

void Engine::Rigidbody::OnActivation()
{
    _inverseMass = (_mass > 0.0f) ? 1.0f / _mass : 0.0f;
}

void Engine::Rigidbody::ApplyForce(const glm::vec2 force)
{
    _acceleration += force * _inverseMass;
}

void Engine::Rigidbody::ApplyImpulse(glm::vec2 impulse)
{
    _velocity += impulse * _inverseMass;
}

void Engine::Rigidbody::Update(const float deltaTime)
{
    _velocity += _acceleration * deltaTime;
    gameObject->transform.position += _velocity * deltaTime;
    _acceleration = glm::vec2(0.0f); // Reset acceleration after integration
}
