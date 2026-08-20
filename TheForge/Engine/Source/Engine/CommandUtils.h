#pragma once
#include <string>

struct CommandUtils
{
    // Non-throwing. Returns false when the string is not a boolean, leaving `out`
    // untouched.
    //
    // Prefer this in command handlers. A handler runs inside an ImGui frame with
    // nothing between it and the render loop that catches, so throwing at a user who
    // mistyped an argument takes the whole process down -- which is exactly what bare
    // `/debug` used to do.
    static bool TryParseBoolean(const std::string& str, bool& out);

    // Throws std::invalid_argument when the string is not a boolean. Kept for callers
    // that already handle it; new code should use TryParseBoolean.
    static bool ParseBoolean(const std::string& str);
};
