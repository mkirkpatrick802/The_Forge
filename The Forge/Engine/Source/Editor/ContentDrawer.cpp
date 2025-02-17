#include "ContentDrawer.h"

#include <fstream>

#include "DetailsEditor.h"
#include "Engine/GameObject.h"
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
    _spriteIcon = CreateTexture("Assets/Engine Assets/Sprites/Editor/SpriteIcon.png", Engine::Texture::TextureType::PIXEL);
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

    ImGui::SameLine();

    // Right Panel: Thumbnail Grid
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    DrawDirectoryContents();
    ImGui::EndChild();

    ImGui::End();
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

    for (auto& child : selectedNode.children)
    {
        auto icon = child.isDirectory ? _folderIcon : _fileIcon;
        if (child.name.ends_with(".png") || child.name.ends_with(".jpg"))
            icon = _spriteIcon;

        // Create a selectable button-like image to enable drag
        ImGui::PushID(child.name.c_str()); // Ensure unique ID for each item
        if (ImGui::ImageButton((void*)(intptr_t)icon->GetID(), ImVec2(_thumbnailSize, _thumbnailSize)))
        {
            if (child.isDirectory)
                _currentDirectory = child.fullPath;

            // Edit prefab
            if (child.name.ends_with(".prefab"))
            {
                std::ifstream prefab(child.fullPath);
                auto go = DEBUG_NEW Engine::GameObject();
                
                nlohmann::json data;
                prefab >> data;
                prefab.close();
                
                go->Deserialize(data);
                go->isPrefab = true;
                go->filepath = child.fullPath;
                
                DetailsEditor::SetSelectedGameObject(go);
            }
        }

        // Begin drag operation from the image button
        if (ImGui::IsItemActive() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("FILE_PATH", child.fullPath.c_str(), child.fullPath.size() + 1); // Use full path for proper loading
            ImGui::Text("Dragging: %s", child.name.c_str()); // Optional: Display drag info
            ImGui::EndDragDropSource();
        }
        
        // Get the available width for text wrapping
        float availableWidth = _thumbnailSize; // Use the same size as the icon

        // Enable text wrapping
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + availableWidth);
        ImGui::TextUnformatted(child.name.c_str());
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
