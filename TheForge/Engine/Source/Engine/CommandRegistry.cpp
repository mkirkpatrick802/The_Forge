#include "CommandRegistry.h"

#include <algorithm>
#include <ranges>

#include "System.h"

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
    auto it = _commands.find(command);

    // Commands are registered with a slash, but the server console is typed at without
    // one. Rather than make every caller know the convention, try the other form.
    if (it == _commands.end())
        it = _commands.find(command.starts_with('/') ? command.substr(1) : '/' + command);

    if (it == _commands.end())
        return false;

    // A command handler is arbitrary code running inside an ImGui frame, and nothing
    // between here and the render loop catches. Containing it here means a handler that
    // throws reports a failed command instead of terminating the process -- which is
    // what /debug with no argument used to do.
    try
    {
        it->second(args);
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG("Command '%s' failed: %s", command.c_str(), e.what())
    }

    return true;
}

void Engine::CommandRegistry::UnregisterCommand(const std::string& name)
{
    _commands.erase(name);
}

std::vector<std::string> Engine::CommandRegistry::GetCommandNames()
{
    std::vector<std::string> names;
    names.reserve(_commands.size());

    for (const auto& name : _commands | std::views::keys)
        names.push_back(name);

    std::ranges::sort(names);
    return names;
}
