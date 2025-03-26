#include "Rigidbody.h"

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "CircleCollider.h"
#include "imgui.h"
#include "RectangleCollider.h"
#include "Engine/Collisions/Collider.h"

Engine::Rigidbody::Rigidbody(): _velocity(), _acceleration(), _useAreaAsMass(true), _mass(10), _density(1),
                                _inverseMass(0),
                                _static(false), _friction(0)
{
}

void Engine::Rigidbody::OnActivation()
{
    
}

void Engine::Rigidbody::Start()
{
    if (const auto collider = gameObject->GetComponent<Collider>(); collider && _useAreaAsMass)
    {
        if (collider->GetColliderType() == EColliderType::ECT_Circle)
        {
            const auto circle = dynamic_cast<CircleCollider*>(collider);
            _mass = (glm::pi<float>() * (circle->GetRadius() * circle->GetRadius())) * _density;
        }
        else if (collider->GetColliderType() == EColliderType::ECT_Rectangle)
        {
            const auto rectangle = dynamic_cast<RectangleCollider*>(collider);
            _mass = (rectangle->GetSize().x * rectangle->GetSize().y) * _density;
        }
    }

    _inverseMass = (_mass > 0.0f) ? 1.0f / _mass : 0.0f;
}

void Engine::Rigidbody::ApplyForce(const glm::vec2 force)
{
    _acceleration += force * _inverseMass;
}

void Engine::Rigidbody::ApplyImpulse(const glm::vec2 impulse)
{
    _velocity += impulse * _inverseMass;
}

void Engine::Rigidbody::Update(const float deltaTime)
{
    _velocity += _acceleration * deltaTime;
    if(length(_velocity) > std::numeric_limits<float>::epsilon())
    {
        gameObject->SetPosition(gameObject->GetWorldPosition() + _velocity * deltaTime);
        _velocity -= _velocity * _friction;
        _acceleration = glm::vec2(0.0f); // Reset acceleration after integration
    }
}

void Engine::Rigidbody::DrawDetails()
{
    ImGui::Checkbox("Is Static", &_static);
    ImGui::Checkbox("Use Area & Density As Mass", &_useAreaAsMass);
    if (!_useAreaAsMass)
    {
        ImGui::InputFloat("Mass", &_mass);
    }
    else
    {
        ImGui::InputFloat("Density", &_density);
    }
}

nlohmann::json Engine::Rigidbody::Serialize()
{
    nlohmann::json data =  Component::Serialize();
    data["Use Area As Mass"] = _useAreaAsMass;
    data["Density"] = _density;
    data["Mass"] = _mass;
    data["Static"] = _static;
    return data;
}

void Engine::Rigidbody::Deserialize(const nlohmann::json& json)
{
    Component::Deserialize(json);

    if (json.contains("Use Area As Mass"))
        _useAreaAsMass = json["Use Area As Mass"];

    if (json.contains("Density"))
        _density = json["Density"];
    
    if (json.contains("Mass"))
        _mass = json["Mass"];

    if (json.contains("Static"))
        _static = json["Static"];
}

void Engine::Rigidbody::Write(NetCode::OutputByteStream& stream) const
{
    Component::Write(stream);

    stream.Write(_velocity);
    stream.Write(_acceleration);
}

void Engine::Rigidbody::Read(NetCode::InputByteStream& stream)
{
    Component::Read(stream);

    stream.Read(_velocity);
    stream.Read(_acceleration);
}
