#pragma once
#include "Component.h"
#include "EventDispatcher.h"

class GameObjectManager;

class Collider : public Component, public EventDispatcher
{
public:
	const static uint32 ComponentID = COLLIDER;

	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

	// Returns true if collider is hitting another collider
	bool CheckCollision();

private:

	GameObjectManager* _objectManager = nullptr;
	Vector2D size = Vector2D();
	bool sweep = false;
};
