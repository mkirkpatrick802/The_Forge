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
        
        String _name;

    public:

        String GetName() const { return _name; }
        
    };
}
