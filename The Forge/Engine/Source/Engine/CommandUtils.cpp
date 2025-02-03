#include "CommandUtils.h"

#include <algorithm>
#include <stdexcept>

bool CommandUtils::ParseBoolean(const std::string& str)
{
    std::string input = str;
    std::ranges::transform(input, input.begin(), ::tolower);

    // Trim leading and trailing whitespace
    input.erase(0, input.find_first_not_of(" \t\n\r"));
    input.erase(input.find_last_not_of(" \t\n\r") + 1);

    // Check for valid boolean representations
    if (input == "true" || input == "1") {
        return true;
    }
    if (input == "false" || input == "0") {
        return false;
    }

    // If the string does not represent a valid boolean, throw an exception or return a default
    throw std::invalid_argument("Invalid boolean string: " + str);
}
