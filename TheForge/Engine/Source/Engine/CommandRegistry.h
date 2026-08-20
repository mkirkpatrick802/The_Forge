#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
    // This is a static class that holds all commands.
    // Any object may add new commands to this registry, which can then be called by the command terminal
    //
    // Commands are registered *with* their leading slash ("/debug"), because the chat
    // window passes the typed line through untouched. ExecuteCommand accepts either
    // form, so a caller with no keyboard convention -- the headless server console --
    // can pass a bare name.
    class CommandRegistry
    {
    public:
        static void RegisterCommand(const std::string& name, const std::function<void(const std::string&)>& function);
        static bool ExecuteCommand(const std::string& input);

        static void UnregisterCommand(const std::string& name);

        // Every registered command, sorted. Lets help enumerate what actually exists
        // rather than being a hand-maintained list that drifts.
        static std::vector<std::string> GetCommandNames();

    private:

        static std::unordered_map<std::string, std::function<void(const std::string&)>> _commands;
    };
}
