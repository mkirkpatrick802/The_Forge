#pragma once
#include "Component.h"

class Health : public Component
{
public:
	const static uint32 ComponentID = HEALTH;

	virtual void LoadData(const json& data) override;

	void TakeDamage(float damage, const GameObject* cause = nullptr);

	float GetCurrentHealth() const { return _currentHealth; }
	float GetMaxHealth() const { return _maxHealth; }

	void SetCurrentHealth(const float health) { _currentHealth = health; }
	virtual void OnDestroyed() override;

private:

	float _maxHealth = 0;
	float _currentHealth = 0;

};