#pragma once
#include "Component.h"

class Projectile : public Component
{
public:

	const static uint32 ComponentID = PROJECTILE;

	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

	void DeathTimer(float deltaTime);

public:

	float speed;
	float lifetime = 5;

private:

	float _lifetimeElapsed = 0;

};