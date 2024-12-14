#include "CommandRegistry.h"

std::unordered_map<String, std::function<void(const String&)>> Engine::CommandRegistry::_commands;

void Engine::CommandRegistry::RegisterCommand(const String& name, const std::function<void(const String&)>& function)
{
    _commands[name] = function;
}

bool Engine::CommandRegistry::ExecuteCommand(const String& input)
{
    // Split command and arguments
    size_t spacePos = input.find(' ');
    String command = input.substr(0, spacePos);
    String args = (spacePos != String::npos) ? input.substr(spacePos + 1) : "";

    // Find and execute the command
    if (const auto it = _commands.find(command); it != _commands.end())
    {
        it->second(args);
        return true;
    }

    return false;
}
