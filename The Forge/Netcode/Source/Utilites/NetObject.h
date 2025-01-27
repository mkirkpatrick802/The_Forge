#pragma once
// DO NOT INCLUDE ANY STEAM API HEADERS!!!!

namespace NetCode
{
    class NetObject
    {
    public:
        NetObject() = default;
        virtual ~NetObject() = default;

        virtual void Update();

    private:
        
    };
}
