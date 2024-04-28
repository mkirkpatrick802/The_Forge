#pragma once
#include "Component.h"

class EnemyController : public Component
{
public:
	const static uint32 ComponentID = ENEMY_CONTROLLER;

	virtual void LoadData(const json& data) override;

	virtual void BeginPlay() override;
	virtual void Update(float deltaTime) override;

private:

	void FindTarget();
	void MoveToTarget();

	void ColliderHit(const GameObject* hit);

private:

	float _speed;
	float _damage;

	GameObjectManager* _objectManager = nullptr;
	GameObject* _target = nullptr;

};