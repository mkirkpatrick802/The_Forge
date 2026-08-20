#include "CommandUtils.h"

#include <algorithm>
#include <stdexcept>

bool CommandUtils::TryParseBoolean(const std::string& str, bool& out)
{
    std::string input = str;
    std::ranges::transform(input, input.begin(), ::tolower);

    // Trim leading and trailing whitespace. Order matters: trimming the front first
    // means an all-whitespace string is already empty by the second call, whose
    // find_last_not_of returns npos and wraps to 0 -- correct, but only by accident,
    // so the empty case is handled up front rather than relied on.
    input.erase(0, input.find_first_not_of(" \t\n\r"));
    if (input.empty()) return false;

    input.erase(input.find_last_not_of(" \t\n\r") + 1);

    if (input == "true" || input == "1")
    {
        out = true;
        return true;
    }

    if (input == "false" || input == "0")
    {
        out = false;
        return true;
    }

    return false;
}

bool CommandUtils::ParseBoolean(const std::string& str)
{
    if (bool value; TryParseBoolean(str, value))
        return value;

    throw std::invalid_argument("Invalid boolean string: " + str);
}
