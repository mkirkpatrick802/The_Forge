#include "UIActionRegistry.h"

#include <algorithm>
#include <ranges>

#include "Engine/System.h"

std::unordered_map<std::string, std::function<void()>>& Engine::UIActionRegistry::Actions()
{
    // Function-local, so registration from a static initialiser cannot run before the
    // map is constructed. The same reason GameModeRegistry does it this way.
    static std::unordered_map<std::string, std::function<void()>> actions;
    return actions;
}

void Engine::UIActionRegistry::Register(const std::string& name, std::function<void()> action)
{
    if (name.empty() || !action) return;

    Actions()[name] = std::move(action);
}

bool Engine::UIActionRegistry::Has(const std::string& name)
{
    return Actions().contains(name);
}

bool Engine::UIActionRegistry::Invoke(const std::string& name)
{
    const auto action = Actions().find(name);
    if (action == Actions().end())
    {
        // Said out loud. A button whose action name is misspelled is otherwise a button
        // that silently does nothing, which looks like a broken click rather than a
        // typo in a level file.
        DEBUG_LOG("UI: no action registered under '%s'.", name.c_str())
        return false;
    }

    action->second();
    return true;
}

std::vector<std::string> Engine::UIActionRegistry::GetNames()
{
    std::vector<std::string> names;
    names.reserve(Actions().size());

    for (const auto& name : Actions() | std::views::keys)
        names.push_back(name);

    std::ranges::sort(names);

    return names;
}
