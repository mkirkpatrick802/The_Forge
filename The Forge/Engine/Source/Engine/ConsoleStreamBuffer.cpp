#include "ConsoleStreamBuffer.h"
#include "System.h"

int Engine::ConsoleStreamBuffer::overflow(int c)
{
    if (c != EOF) {
        _buffer += static_cast<char>(c);
    }

    // If we hit a newline, log the accumulated string
    if (c == '\n') {
        System::LogToConsole("%s", _buffer.c_str());  // Log the full string to the console
        _buffer.clear();  // Clear the buffer for the next message
    }

    return c;
}

int Engine::ConsoleStreamBuffer::sync()
{
    if (!_buffer.empty()) {
        System::LogToConsole("%s", _buffer.c_str());  // Log any remaining message
        _buffer.clear();
    }
    return 0;
}

void Engine::ConsoleStreamBuffer::CleanUp()
{
    _buffer.clear();
    _buffer.shrink_to_fit();
}
