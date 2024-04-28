#include "Projectile.h"

#include "Collider.h"
#include "GameObject.h"
#include "Health.h"

void Projectile::LoadData(const json& data)
{
	_speed = data["Speed"];
	_damage = data["Damage"];
}

void Projectile::BeginPlay()
{
	Component::BeginPlay();

	if (gameObject->GetComponent<Collider>())
		gameObject->GetComponent<Collider>()->RegisterHitCallback([this](const GameObject* hit) { this->ColliderHit(hit); });
}

// TODO: Make the server handle this
void Projectile::Update(const float deltaTime)
{
	Component::Update(deltaTime);

	const float radians = glm::radians(gameObject->transform.rotation + 90);
	const float xAxis = std::cos(radians);
	const float yAxis = std::sin(radians);

	const Vector2D movementVector = glm::normalize(Vector2D(xAxis * -1, yAxis)) * _speed;

	gameObject->transform.position = movementVector + gameObject->GetPosition();

	PositionCheck();
}

void Projectile::PositionCheck()
{
	Vector2D position = gameObject->GetPosition();
	if (position.x > 1000 || position.x < -1000)
		gameObject->Destroy();

	if (position.y > 1000 || position.y < -1000)
		gameObject->Destroy();
}

void Projectile::ColliderHit(const GameObject* hit)
{
	if (Health* health = hit->GetComponent<Health>())
		health->TakeDamage(_damage, gameObject->owner);

	gameObject->Destroy();
}