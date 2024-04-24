#pragma once
#include "Component.h"

class Health : public Component
{
public:
	const static uint32 ComponentID = HEALTH;

	virtual void LoadData(const json& data) override;

	void TakeDamage(float damage);


private:

	float _maxHealth = 0;
	float _currentHealth = 0;

};