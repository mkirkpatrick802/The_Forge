#pragma once
#include <string>
#include <glm/vec2.hpp>

namespace Engine
{
    class ImGuiHelper
    {
    public:
        static bool CenteredButtonWithPadding(const std::string& label, float verticalPadding);
        static bool DragDropFileButton(const std::string& label, std::string& filePath, const char* payloadType);
        static void DisplayFilePath(const std::string& label, const std::string& filePath);
        static void InputVector2(const std::string& label, glm::vec2& input);
    };
}
