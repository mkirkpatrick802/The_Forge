#pragma once
#include "Component.h"
#include "EventListener.h"

class Projectile : public Component, public EventListener
{
public:

	const static uint32 ComponentID = PROJECTILE;

	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

	virtual void OnEvent(Event* event) override;

	void DeathTimer(float deltaTime);

	float speed;
	float lifetime = 5;

private:

	float _lifetimeElapsed = 0;

};