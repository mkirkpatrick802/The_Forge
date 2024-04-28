#include "EnemyController.h"

#include "Client.h"
#include "Collider.h"
#include "GameObjectManager.h"
#include "Health.h"
#include "ObjectStateWriter.h"
#include "PlayerController.h"
#include "Server.h"

void EnemyController::LoadData(const json& data)
{
	_speed = data["Speed"];
	_damage = data["Damage"];
}

void EnemyController::BeginPlay()
{
	Component::BeginPlay();

	_objectManager = GameObjectManager::GetInstance();

	if (gameObject->GetComponent<Collider>())
		gameObject->GetComponent<Collider>()->RegisterHitCallback([this](const GameObject* hit) { this->ColliderHit(hit); });
}

void EnemyController::Update(const float deltaTime)
{
	Component::Update(deltaTime);

	FindTarget();
	MoveToTarget();
}

void EnemyController::FindTarget()
{
	if (_objectManager)
	{
		auto players = _objectManager->GetGameObjectsWithComponent<PlayerController>();

		if (players.empty())
			_target = nullptr;

		float shortestDistance = 10000;
		Vector2D position = gameObject->GetPosition();
		for (auto player : players)
			if (float distance = length(player->GetPosition() - position); distance < shortestDistance)
			{
				shortestDistance = distance;
				_target = player;
			}
	}
}

void EnemyController::MoveToTarget()
{
	if(!_target) return;
	if(gameObject->GetPosition() == _target->GetPosition()) return;

	gameObject->transform.position += normalize(_target->GetPosition() - gameObject->GetPosition()) * _speed;

	/*if(!Client::IsHostClient())
	{
		ByteStream stream;
		stream.WriteGSM(GSM_Client::GSM_UpdateObjectRequest);
		stream.AppendToBuffer(gameObject->instanceID);
		Client::SendByteStreamToServer(stream);
	}*/
}

void EnemyController::ColliderHit(const GameObject* hit)
{
	if(hit->GetComponent<PlayerController>())
	{
		hit->GetComponent<Health>()->TakeDamage(_damage, gameObject);
		gameObject->Destroy();
	}
}
