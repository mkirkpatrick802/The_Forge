#pragma once
#include "Component.h"

class Projectile : public Component
{
public:

	const static uint32 ComponentID = PROJECTILE;

	Projectile();
	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

private:

};