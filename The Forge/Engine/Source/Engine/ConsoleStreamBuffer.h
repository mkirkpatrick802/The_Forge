#pragma once
#include <streambuf>

namespace Engine
{
    class ConsoleStreamBuffer : public std::streambuf
    {
    public:
        ConsoleStreamBuffer() = default;
        ~ConsoleStreamBuffer() override = default;

        void CleanUp();
        
    protected:
        
        int overflow(int c) override;
        int sync() override;

    private:
        
        std::string _buffer;
    };
}
