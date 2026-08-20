#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
    // What an authored button does, looked up by name.
    //
    // A UIWidget laid out in the editor stores a string like "Play" or "Quit"; this is
    // what turns that string into behaviour. The indirection is the point: a level file
    // is data, and data must not be able to name arbitrary code. A name that is not
    // registered does nothing and says so, rather than being a way for a level to reach
    // into the engine.
    //
    // Same shape as GameModeRegistry and CommandRegistry, for the same reason: the
    // engine provides the mechanism, the game registers what its own buttons mean.
    class UIActionRegistry
    {
    public:
        static void Register(const std::string& name, std::function<void()> action);

        // Runs the named action. Returns false if nothing is registered under that name,
        // which the caller should treat as an authoring mistake rather than ignore.
        static bool Invoke(const std::string& name);

        static bool Has(const std::string& name);

        // Every registered name, sorted. Lets the details panel offer what actually
        // exists rather than a hand-maintained list that drifts from it.
        static std::vector<std::string> GetNames();

    private:
        static std::unordered_map<std::string, std::function<void()>>& Actions();
    };
}
