#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "json.hpp"

namespace Engine
{
    enum class ECollisionObjectType : uint8_t
    {
        ECOT_Default = 0,
        ECOT_PhysicsBody,
        ECOT_Player,
        ECOT_Projectile,
        ECOT_Walkable,
        ECOT_Max
    };
    
    // Convert CollisionObjectType to string for UI/debugging
    inline std::string ToString(ECollisionObjectType type)
    {
        switch (type) {
            case ECollisionObjectType::ECOT_Default: return "Default";
            case ECollisionObjectType::ECOT_PhysicsBody: return "Physics Body";
            case ECollisionObjectType::ECOT_Player: return "Player";
            case ECollisionObjectType::ECOT_Projectile: return "Projectile";
            case ECollisionObjectType::ECOT_Walkable: return "Walkable";
        }
    
        return "Unknown";
    }
    
    enum class ECollisionResponse : uint8_t
    {
        ECR_Ignore = 0,
        ECR_Overlap,
        ECR_Block
    };
    
    // Default response map shared across all profiles
    inline std::unordered_map<ECollisionObjectType, ECollisionResponse> GetDefaultResponseMap()
    {
        return {
            { ECollisionObjectType::ECOT_Default, ECollisionResponse::ECR_Block },
            { ECollisionObjectType::ECOT_PhysicsBody, ECollisionResponse::ECR_Overlap },
            { ECollisionObjectType::ECOT_Player, ECollisionResponse::ECR_Block },
            { ECollisionObjectType::ECOT_Projectile, ECollisionResponse::ECR_Overlap },
            { ECollisionObjectType::ECOT_Walkable, ECollisionResponse::ECR_Overlap }
        };
    }
    
    struct CollisionProfile
    {
        ECollisionObjectType type;
        std::unordered_map<ECollisionObjectType, ECollisionResponse> responseMap;
    
        // Constructor with default responses
        explicit CollisionProfile(const ECollisionObjectType objectType = ECollisionObjectType::ECOT_Default)
            : type(objectType), responseMap(GetDefaultResponseMap()) {}
    
        // Get collision response with a fallback to Ignore
        ECollisionResponse GetResponse(const ECollisionObjectType other) const
        {
            const auto it = responseMap.find(other);
            return (it != responseMap.end()) ? it->second : ECollisionResponse::ECR_Ignore;
        }
    
        // Set response dynamically
        void SetResponse(const ECollisionObjectType other, const ECollisionResponse response)
        {
            responseMap[other] = response;
        }
    
        static ECollisionResponse ResolveCollision(const CollisionProfile& a, const CollisionProfile& b) 
        {
            const ECollisionResponse responseA = a.GetResponse(b.type);
            const ECollisionResponse responseB = b.GetResponse(a.type);
    
            // The lowest value takes priority: Ignore (0) < Overlap (1) < Block (2)
            return (responseA < responseB) ? responseA : responseB;
        }
    };
    
        NLOHMANN_JSON_SERIALIZE_ENUM(ECollisionObjectType,
    {
        {ECollisionObjectType::ECOT_Default, "Default"},
        {ECollisionObjectType::ECOT_PhysicsBody, "PhysicsBody"},
        {ECollisionObjectType::ECOT_Player, "Player"},
        {ECollisionObjectType::ECOT_Projectile, "Projectile"},
        {ECollisionObjectType::ECOT_Walkable, "Walkable"}
    })
    
    NLOHMANN_JSON_SERIALIZE_ENUM(ECollisionResponse,
    {
        {ECollisionResponse::ECR_Ignore, "Ignore"},
        {ECollisionResponse::ECR_Overlap, "Overlap"},
        {ECollisionResponse::ECR_Block, "Block"}
    })
    
    // Convert CollisionProfile to JSON
    inline void to_json(nlohmann::json& j, const CollisionProfile& profile)
    {
        j = nlohmann::json{
            {"type", profile.type},
            {"responses", profile.responseMap}
        };
    }
    
    // Convert JSON to CollisionProfile
    // Convert JSON to CollisionProfile
    inline void from_json(const nlohmann::json& j, CollisionProfile& profile)
    {
        j.at("type").get_to(profile.type);
        
        // Check if "responses" is present in the JSON, if not, use the default response map
        if (j.contains("responses"))
        {
            j.at("responses").get_to(profile.responseMap);
        }
        else
        {
            // Set the responseMap to the default map if not provided in the JSON
            profile.responseMap = GetDefaultResponseMap();
        }
        
        // Ensure all object types have an entry in the responseMap
        for (uint8_t i = 0; i < static_cast<uint8_t>(ECollisionObjectType::ECOT_Max); ++i)
        {
            if (auto type = static_cast<ECollisionObjectType>(i); !profile.responseMap.contains(type))
            {
                // Add missing object types with default response
                profile.responseMap[type] = ECollisionResponse::ECR_Block;
            }
        }
    }
}
    
    