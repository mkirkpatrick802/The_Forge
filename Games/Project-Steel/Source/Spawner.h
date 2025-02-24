#pragma once
#include <random>

#include "Engine/Components/ComponentUtils.h"

class Spawner : public Engine::Component
{
public:
    Spawner();
    void Start() override;
    void Update(float deltaTime) override;

    nlohmann::json Serialize() override;
    void Deserialize(const nlohmann::json& json) override;
    void DrawDetails() override;

private:
    void SpawnObject();
    
private:
    std::string _prefabToSpawn = "";
    float _spawnTime;
    float _timeElapsed;
    float _radius;
    int _initialAmount;

    std::random_device _rd;    // Non-deterministic random device
    std::mt19937 _gen;         // Mersenne Twister engine for better random numbers
    std::uniform_real_distribution<float> _dist;  // Uniform distribution for random float
};

REGISTER_COMPONENT(Spawner)
