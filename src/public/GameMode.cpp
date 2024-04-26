#include "GameMode.h"

#include <cstdlib>

#include "EnemySpawner.h"
#include <EnemySpawnedEvent.h>

void GameMode::BeginPlay()
{

}

void GameMode::Update(const float deltaTime)
{
    // Define the interval range for spawning enemies(in seconds)
	float minSpawnInterval = 5.0f; // Minimum interval
    float maxSpawnInterval = 10.0f; // Maximum interval

    // Generate a random spawn interval within the defined range
    float spawnInterval = minSpawnInterval + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxSpawnInterval - minSpawnInterval)));

    // Update the timer
    static float timer = 0.0f;
    timer += deltaTime;

    // Check if it's time to spawn a new enemy
    if (timer >= spawnInterval)
    {
        // Reset the timer
        timer = 0.0f;

        // Spawn an enemy
        GameObject* spawnedEnemy = EnemySpawner::SpawnEnemy();

        // Notify event
        EnemySpawnedEvent* event = CreateEvent<EnemySpawnedEvent>();
        event->spawned = spawnedEnemy;
        Notify(event);
    }
}
