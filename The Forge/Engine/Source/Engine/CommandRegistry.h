#pragma once
#include <functional>
#include <unordered_map>

#include "Data.h"

namespace Engine
{
    // This is a static class that holds all commands.
    // Any object may add new commands to this registry, which can then be called by the command terminal
    class CommandRegistry
    {
    public:
        static void RegisterCommand(const String& name, const std::function<void(const String&)>& function);
        static bool ExecuteCommand(const String& input);

        static void UnregisterCommand(const String& name);
    private:

        static std::unordered_map<String, std::function<void(const String&)>> _commands;
    };
}
