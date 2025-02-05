#include "ContentDrawer.h"

#include "Engine/Rendering/TextureLoader.h"

Editor::ContentDrawer::ContentDrawer()
{
    _loadTextures = true;
}

void Editor::ContentDrawer::LoadTextures()
{
    _emptyFolderIcon = CreateTexture("Assets/Sprites/Editor/EmptyFolderIcon.png", Engine::Texture::TextureType::PIXEL);
    _folderIcon = CreateTexture("Assets/Sprites/Editor/FolderIcon.png", Engine::Texture::TextureType::PIXEL);
    _fileIcon = CreateTexture("Assets/Sprites/Editor/FileIcon.png", Engine::Texture::TextureType::PIXEL);
    _spriteIcon = CreateTexture("Assets/Sprites/Editor/SpriteIcon.png", Engine::Texture::TextureType::PIXEL);
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

    // Left Panel: Directory Tree
    /*ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true);
    ImGui::Text("File Structure:");
    DrawFileTree(root);
    ImGui::EndChild();*/

    ImGui::SameLine();

    // Right Panel: Thumbnail Grid
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    DrawDirectoryContents();
    ImGui::EndChild();

    ImGui::End();
}

void Editor::ContentDrawer::DrawFileTree(const FileNode& node)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (_currentDirectory == node.fullPath) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (node.isDirectory) {
        bool opened = ImGui::TreeNodeEx(node.name.c_str(), flags);

        if (ImGui::IsItemClicked()) {
            _currentDirectory = node.fullPath;
        }

        if (opened) {
            for (auto& child : node.children) {
                DrawFileTree(child);
            }
            ImGui::TreePop();
        }
    }
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

    int columns = std::max(1, int(ImGui::GetContentRegionAvail().x / (_thumbnailSize + 16)));
    ImGui::Columns(columns, nullptr, false);

    for (auto& child : selectedNode.children)
    {

        auto icon = child.isDirectory ? _folderIcon : _fileIcon;
        if (child.name.ends_with(".png") || child.name.ends_with(".jpg"))
            icon = _spriteIcon;

        ImGui::Image((void*)(intptr_t)icon->GetID(), ImVec2(_thumbnailSize, _thumbnailSize));

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            if (child.isDirectory) 
                _currentDirectory = child.fullPath;
            

        ImGui::TextWrapped(child.name.c_str());
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
