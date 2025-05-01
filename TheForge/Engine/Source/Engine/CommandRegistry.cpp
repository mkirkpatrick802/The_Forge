#include "CommandRegistry.h"

std::unordered_map<std::string, std::function<void(const std::string&)>> Engine::CommandRegistry::_commands;

void Engine::CommandRegistry::RegisterCommand(const std::string& name, const std::function<void(const std::string&)>& function)
{
    _commands[name] = function;
}

bool Engine::CommandRegistry::ExecuteCommand(const std::string& input)
{
    // Split command and arguments
    size_t spacePos = input.find(' ');
    std::string command = input.substr(0, spacePos);
    std::string args = (spacePos != std::string::npos) ? input.substr(spacePos + 1) : "";

    // Find and execute the command
    if (const auto it = _commands.find(command); it != _commands.end())
    {
        it->second(args);
        return true;
    }

    return false;
}

void Engine::CommandRegistry::UnregisterCommand(const std::string& name)
{
    _commands.erase(name);
}
