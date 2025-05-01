#pragma once
#include <functional>
#include <string>
#include <unordered_map>

namespace Engine
{
    // This is a static class that holds all commands.
    // Any object may add new commands to this registry, which can then be called by the command terminal
    class CommandRegistry
    {
    public:
        static void RegisterCommand(const std::string& name, const std::function<void(const std::string&)>& function);
        static bool ExecuteCommand(const std::string& input);

        static void UnregisterCommand(const std::string& name);
    private:

        static std::unordered_map<std::string, std::function<void(const std::string&)>> _commands;
    };
}
