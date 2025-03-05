#pragma once
#include <cstdint>

enum class ECollisionObjectTypes : uint8_t
{
    ECOT_Player,
    ECOT_PhysicsBody,
    //ECOT_
};

enum class ECollisionResponses : uint8_t
{
    ECR_Ignore,
    ECR_Overlap,
    ECR_Block
};
