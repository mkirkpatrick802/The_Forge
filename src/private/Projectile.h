#pragma once
#include "Component.h"

class Projectile : public Component
{
public:

	const static uint32 ComponentID = PROJECTILE;

	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

private:

	void PositionCheck();

	void ColliderHit(const GameObject* hit);

	float _speed = 0;
	float _damage = 0;
};