#pragma once
#include <filesystem>
#include <vector>

#include "Engine/Rendering/UIWindow.h"

namespace Engine
{
    class Texture;
}

namespace Editor
{
    struct FileNode {
        std::string name;
        std::string fullPath;
        bool isDirectory;
        std::vector<FileNode> children = std::vector<FileNode>();
    };
    
    class ContentDrawer : public Engine::UIWindow
    {
    public:

        ContentDrawer();
        ~ContentDrawer();
        void Render() override;

    private:

        void LoadTextures();
        void DrawDirectoryContents();
        void ScanDirectory(const std::filesystem::path& directory, FileNode& node);

    private:

        std::string _currentDirectory = "Assets";
        float _thumbnailSize = 64.0f;

        bool _loadTextures = true;
        std::shared_ptr<Engine::Texture> _emptyFolderIcon;
        std::shared_ptr<Engine::Texture> _folderIcon;
        std::shared_ptr<Engine::Texture> _fileIcon;
        std::shared_ptr<Engine::Texture> _spriteIcon;

    };
}
