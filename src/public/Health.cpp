#include "Health.h"

#include "ByteStream.h"
#include "Client.h"
#include "EnemyController.h"
#include "GameObject.h"
#include "ObjectStateWriter.h"
#include "PlayerController.h"
#include "ScoreManager.h"
#include "Server.h"

void Health::LoadData(const json& data)
{
	_maxHealth = data["Max Health"];
	_currentHealth = _maxHealth;
}

void Health::TakeDamage(const float damage, const GameObject* cause)
{
	_currentHealth -= damage;
	if (_currentHealth <= 0)
	{
		if (cause != nullptr)
			if (cause->GetComponent<PlayerController>())
				if (gameObject->GetComponent<EnemyController>())
					ScoreManager::AddScore(cause->GetComponent<PlayerController>()->playerID);

		gameObject->Destroy();
		return;
	}

	if(Client::IsHostClient())
		ObjectStateWriter::UpdateObjectState(gameObject, true);
}

void Health::OnDestroyed()
{
	Component::OnDestroyed();


}
