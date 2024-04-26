#include "Health.h"

#include "GameObject.h"

void Health::LoadData(const json& data)
{
	_maxHealth = data["Max Health"];
	_currentHealth = _maxHealth;
}

void Health::TakeDamage(const float damage)
{
	_currentHealth -= damage;
	if (_currentHealth > 0) return;

	printf("%s Destroyed \n", gameObject->name.c_str());
	gameObject->Destroy();
}