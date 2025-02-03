#pragma once
#include "json.hpp"
#include "Transform.h"

namespace Engine
{
    
    class Object
    {
        friend class Level;
    public:
        
    protected:

        virtual nlohmann::json SaveObject();
        
    public:

        Transform transform;

    protected:
        
        std::string _name;

    public:

        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { _name = name; }
        
    };
}
