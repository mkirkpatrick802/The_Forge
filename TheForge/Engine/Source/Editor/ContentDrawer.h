#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
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

        // Only images carry import metadata -- the sidecar is produced by reading an
        // image header. Everything else has nothing to reimport.
        static bool IsImageAsset(const std::string& name);

        // Runs the reimport requested during the item loop. Deferred so the directory
        // tree is not mutated while it is being iterated, matching how LevelEditor
        // defers its deletes.
        void ProcessPendingImports();

    private:

        std::string _currentDirectory = "Assets";
        float _thumbnailSize = 64.0f;

        bool _loadTextures = true;
        std::shared_ptr<Engine::Texture> _emptyFolderIcon;
        std::shared_ptr<Engine::Texture> _folderIcon;
        std::shared_ptr<Engine::Texture> _fileIcon;

        // Thumbnails, keyed by path. A member rather than a function-local static so a
        // reimported asset can have its entry dropped -- otherwise the drawer keeps
        // showing the image as it was when first opened.
        std::unordered_map<std::string, std::shared_ptr<Engine::Texture>> _thumbnailCache;

        std::string _assetToReimport;
        bool _reimportCurrentFolder = false;
    };
}
