#include "ContentDrawer.h"

#include <fstream>

#include "Engine/GameObject.h"
#include "Engine/LevelManager.h"
#include "Engine/AssetMetadata.h"
#include "Engine/System.h"
#include "Engine/Rendering/TextureLoader.h"

Editor::ContentDrawer::ContentDrawer()
{
    _loadTextures = true;
}

void Editor::ContentDrawer::LoadTextures()
{
    _emptyFolderIcon = CreateTexture("Assets/Engine Assets/Sprites/Editor/EmptyFolderIcon.png", Engine::Texture::TextureType::PIXEL);
    _folderIcon = CreateTexture("Assets/Engine Assets/Sprites/Editor/FolderIcon.png", Engine::Texture::TextureType::PIXEL);
    _fileIcon = CreateTexture("Assets/Engine Assets/Sprites/Editor/FileIcon.png", Engine::Texture::TextureType::PIXEL);
}

Editor::ContentDrawer::~ContentDrawer() = default;

void Editor::ContentDrawer::Render()
{
    if (_loadTextures)
    {
        LoadTextures();
        _loadTextures = false;
    }
    
    FileNode root = { .name= "Assets", .isDirectory= true, .children= std::vector<FileNode>() }; // Root directory name
    ScanDirectory("Assets", root);

    ImGui::Begin("Content Drawer");

    if (ImGui::Button("Reimport All"))
        _reimportCurrentFolder = true;

    ImGui::SameLine();

    // Right Panel: Thumbnail Grid
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    DrawDirectoryContents();
    ImGui::EndChild();

    ImGui::End();

    // After the item loop, never during it.
    ProcessPendingImports();
}

bool Editor::ContentDrawer::IsImageAsset(const std::string& name)
{
    return name.ends_with(".png") || name.ends_with(".jpg") || name.ends_with(".jpeg");
}

void Editor::ContentDrawer::ProcessPendingImports()
{
    if (!_assetToReimport.empty())
    {
        const std::string path = _assetToReimport;
        _assetToReimport.clear();

        if (Engine::ImportImageAsset(path))
        {
            DEBUG_LOG("Reimported %s", path.c_str())

            // Dropped so the thumbnail is rebuilt from the file on disk. Without this a
            // reimport of a changed image keeps showing the old picture.
            _thumbnailCache.erase(path);
        }
    }

    if (!_reimportCurrentFolder) return;
    _reimportCurrentFolder = false;

    int imported = 0;
    int failed = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(_currentDirectory))
    {
        if (entry.is_directory()) continue;

        const std::string path = entry.path().string();
        if (!IsImageAsset(entry.path().filename().string())) continue;

        if (Engine::ImportImageAsset(path))
        {
            ++imported;
            _thumbnailCache.erase(path);
        }
        else
        {
            ++failed;
        }
    }

    DEBUG_LOG("Reimported %d asset(s) under '%s'%s", imported, _currentDirectory.c_str(),
        failed > 0 ? " (some failed, see above)" : "")
}

void Editor::ContentDrawer::DrawDirectoryContents()
{
    FileNode selectedNode;
    ScanDirectory(_currentDirectory, selectedNode);

    // Display navigable path
    std::filesystem::path currentPath = _currentDirectory;
    std::vector<std::filesystem::path> pathParts;

    for (auto it = currentPath.begin(); it != currentPath.end(); ++it)
    {
        pathParts.push_back(*it);
    }

    ImGui::Text("Viewing: ");
    ImGui::SameLine();
    
    std::filesystem::path accumulatedPath;
    for (size_t i = 0; i < pathParts.size(); ++i)
    {
        accumulatedPath /= pathParts[i];

        if (ImGui::Button(pathParts[i].string().c_str()))
            _currentDirectory = accumulatedPath.string();
        
        if (i < pathParts.size() - 1)
        {
            ImGui::SameLine();
            ImGui::Text("/");
            ImGui::SameLine();
        }
    }

    ImGui::Separator();

    int columns = max(1, int(ImGui::GetContentRegionAvail().x / (_thumbnailSize + 16)));
    ImGui::Columns(columns, nullptr, false);

    for (auto& [name, fullPath, isDirectory, children] : selectedNode.children)
    {
        auto icon = isDirectory ? _folderIcon : _fileIcon;
        if (IsImageAsset(name))
        {
            if (!_thumbnailCache.contains(fullPath))
                _thumbnailCache[fullPath] = CreateTexture(fullPath, Engine::Texture::TextureType::PIXEL);

            icon = _thumbnailCache[fullPath];
        }

        // Create a selectable button-like image to enable drag
        ImGui::PushID(name.c_str()); // Ensure unique ID for each item
        if (ImGui::ImageButton((void*)(intptr_t)icon->GetID(), ImVec2(_thumbnailSize, _thumbnailSize)))
        {
            if (isDirectory)
                _currentDirectory = fullPath;

            // Edit prefab.
            //
            // Opened as the world, replacing the level -- the prefab is not built
            // alongside it. Components live in global pools, so a prefab standing next to
            // an open level draws into it, collides with it and can be picked out of the
            // viewport while nothing in the level owns it.
            if (name.ends_with(".prefab"))
                Engine::LevelManager::OpenPrefab(fullPath);
        }

        // Bound to the item above, so it must come before anything else is submitted.
        if (!isDirectory && IsImageAsset(name) && ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Reimport"))
                _assetToReimport = fullPath;

            ImGui::EndPopup();
        }

        // Begin drag operation from the image button
        if (ImGui::IsItemActive() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("FILE_PATH", fullPath.c_str(), fullPath.size() + 1); // Use full path for proper loading
            ImGui::Text("Dragging: %s", name.c_str()); // Optional: Display drag info
            ImGui::EndDragDropSource();
        }
        
        // Get the available width for text wrapping
        float availableWidth = _thumbnailSize; // Use the same size as the icon

        // Enable text wrapping
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + availableWidth);
        ImGui::TextUnformatted(name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::PopID(); // Restore ID stack
        ImGui::NextColumn();
    }


    ImGui::Columns(1);
}

void Editor::ContentDrawer::ScanDirectory(const std::filesystem::path& directory, FileNode& node)
{
    node.fullPath = directory.string();
    node.children.clear();

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        // Sidecars are engine bookkeeping that sits next to the asset it describes.
        // Showing them doubles the apparent contents of every folder once assets are
        // imported, and there is nothing useful to do with one.
        if (entry.path().extension() == ".meta") continue;

        FileNode child;
        child.name = entry.path().filename().string();
        child.fullPath = entry.path().string();
        child.isDirectory = entry.is_directory();
        if (child.isDirectory) {
            ScanDirectory(entry.path(), child);
        }
        node.children.push_back(std::move(child));
    }
}
