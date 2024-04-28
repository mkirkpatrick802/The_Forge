#include "GameMode.h"

#include <cstdlib>

#include "EnemySpawner.h"
#include <EnemySpawnedEvent.h>

#include "ScoreManager.h"

void GameMode::BeginPlay()
{

}

void GameMode::Update(const float deltaTime)
{
    // Define the base interval range for spawning enemies (in seconds)
    float baseMinSpawnInterval = 5.0f;  // Minimum interval at score 0
    float baseMaxSpawnInterval = 10.0f; // Maximum interval at score 0

    int highestScore = ScoreManager::GetHighestScore();

    // Calculate score-dependent spawn intervals
    // As the score increases, decrease intervals to make the game more challenging.
    float scoreFactor = std::max(0.0f, 1.0f - (float)highestScore / 40.0f); // Cap the factor to not go below 0
    float minSpawnInterval = baseMinSpawnInterval * scoreFactor; // Decrease interval as score increases
    float maxSpawnInterval = baseMaxSpawnInterval * scoreFactor;

    // Ensure that the spawn intervals do not fall below a certain threshold
    minSpawnInterval = std::max(minSpawnInterval, 1.0f); // No less than 1 second
    maxSpawnInterval = std::max(maxSpawnInterval, 2.0f); // No less than 2 seconds

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
