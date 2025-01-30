#pragma once
#include <filesystem>
#include <vector>

#include "Engine/Data.h"
#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    struct FileNode {
        String name;
        String fullPath;
        bool isDirectory;
        std::vector<FileNode> children = std::vector<FileNode>();
    };
    
    class ContentDrawer : public Engine::UIWindow
    {
    public:
        void Render() override;

    private:

        void DrawFileTree(const FileNode& node);
        void DrawDirectoryContents(const FileNode& node);
        void ScanDirectory(const std::filesystem::path& directory, FileNode& node);

    private:

        String _currentDirectory = "Assets";
    };
}
