#include "EnemySpawner.h"

#include "GameObjectManager.h"

GameObject* EnemySpawner::SpawnEnemy()
{
    // Get the game object manager instance
    GameObjectManager* manager = GameObjectManager::GetInstance();

    // Define the boundaries of the two rectangles
    // Rectangle 1 (smaller)
    int minX1 = -600; // Minimum x coordinate of rectangle 1
    int maxX1 = 600;  // Maximum x coordinate of rectangle 1
    int minY1 = -300;  // Minimum y coordinate of rectangle 1
    int maxY1 = 300;   // Maximum y coordinate of rectangle 1

    // Rectangle 2 (bigger)
    int minX2 = -700; // Minimum x coordinate of rectangle 2
    int maxX2 = 700;  // Maximum x coordinate of rectangle 2
    int minY2 = -400; // Minimum y coordinate of rectangle 2
    int maxY2 = 400;  // Maximum y coordinate of rectangle 2

    // Generate random x and y coordinates within the bigger rectangle
    int randomX;
    int randomY;

    do 
    {
        randomX = minX2 + rand() % (maxX2 - minX2 + 1);
        randomY = minY2 + rand() % (maxY2 - minY2 + 1);
    } while (randomX >= minX1 && randomX <= maxX1 && randomY >= minY1 && randomY <= maxY1);

    // Create the enemy game object with the random position
    GameObject* enemy = manager->CreateGameObject(ENEMY_PREFAB_ID, Vector2D(randomX, randomY));

    return enemy;
}
