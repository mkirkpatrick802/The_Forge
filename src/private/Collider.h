#pragma once
#include "Component.h"

using HitCallbackFunction = std::function<void(GameObject*)>;

class GameObjectManager;
class Collider : public Component
{
public:
	const static uint32 ComponentID = COLLIDER;

	virtual void LoadData(const json& data) override;
	virtual void BeginPlay() override;

	virtual void Update(float deltaTime) override;

	void RegisterHitCallback(const HitCallbackFunction& function);
	void TriggerHitCallback(GameObject* hit);

private:

	// Returns true if collider is hitting another collider
	bool CheckCollision();

private:

	GameObjectManager* _objectManager = nullptr;
	Vector2D size = Vector2D();
	bool sweep = false;

	HitCallbackFunction callback;
};
