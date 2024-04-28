#include "Health.h"

#include "EnemyController.h"
#include "GameObject.h"
#include "PlayerController.h"
#include "ScoreManager.h"

void Health::LoadData(const json& data)
{
	_maxHealth = data["Max Health"];
	_currentHealth = _maxHealth;
}

void Health::TakeDamage(const float damage, const GameObject* cause)
{
	_currentHealth -= damage;
	if (_currentHealth > 0) return;
	if (cause != nullptr)
		if (cause->GetComponent<PlayerController>())
			if (gameObject->GetComponent<EnemyController>())
				ScoreManager::AddScore(cause->GetComponent<PlayerController>()->playerID);

	printf("%s Destroyed \n", gameObject->name.c_str());
	gameObject->Destroy();
}