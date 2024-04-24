#include "Projectile.h"

#include "Collider.h"
#include "GameObject.h"
#include "ObjectHitEvent.h"

void Projectile::LoadData(const json& data)
{
	speed = data["Speed"];
}

void Projectile::BeginPlay()
{
	SubscribeToEvent(EventType::ET_ObjectHit);

	if (const auto collider = gameObject->GetComponent<Collider>())
		collider->Attach(this);
}

// TODO: Make the server handle this
void Projectile::Update(float deltaTime)
{
	const float radians = glm::radians(gameObject->transform.rotation + 90);
	const float xAxis = std::cos(radians);
	const float yAxis = std::sin(radians);

	const Vector2D movementVector = glm::normalize(Vector2D(xAxis * -1, yAxis)) * speed;

	gameObject->transform.position = movementVector + gameObject->GetPosition();

	DeathTimer(deltaTime);
}

void Projectile::DeathTimer(const float deltaTime)
{
	_lifetimeElapsed += deltaTime;
	if (_lifetimeElapsed < lifetime) return;
	_lifetimeElapsed = 0;

	gameObject->Destroy();
}

void Projectile::OnEvent(Event* event)
{
	switch (event->GetEventType())
	{
	case EventType::ET_ObjectHit:
		{
			auto hitEvent = static_cast<ObjectHitEvent*>(event); //TODO: add virtual function to event class
			hitEvent->hit->Destroy();
			gameObject->Destroy();
		}
		break;
	}
}