#include "Projectile.h"

#include "GameObject.h"

void Projectile::LoadData(const json& data)
{
	speed = data["Speed"];
}

void Projectile::BeginPlay()
{

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