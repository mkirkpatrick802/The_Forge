#include "ContentDrawer.h"

#include <iostream>

void Editor::ContentDrawer::Render()
{
    FileNode root = { .name= "Assets", .isDirectory= true, .children= std::vector<FileNode>() }; // Root directory name
    ScanDirectory("Assets", root);

    ImGui::Begin("Content Drawer");

    // Left Panel (Text-Based View)
    ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, 0), true);
    ImGui::Text("File Structure:");
    DrawFileTree(root);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right Panel (Image-Based View)
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    ImGui::Text(("Viewing: " + _currentDirectory).c_str());

    FileNode selectedNode;
    ScanDirectory(_currentDirectory, selectedNode);
    DrawDirectoryContents(selectedNode);

    ImGui::EndChild();

    ImGui::End();
}

void Editor::ContentDrawer::DrawFileTree(const FileNode& node)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    
    // Highlight selected directory
    if (_currentDirectory == node.fullPath) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (node.isDirectory) {
        bool opened = ImGui::TreeNodeEx(node.name.c_str(), flags);
        
        // Check if the directory is clicked
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

void Editor::ContentDrawer::DrawDirectoryContents(const FileNode& node) {
    for (auto& child : node.children) {
        if (child.isDirectory) {
            if (ImGui::Button(child.name.c_str())) {
                _currentDirectory = child.fullPath;  // Change directory on click
            }
        } else {
            // Placeholder for image preview (Replace with actual texture rendering)
            ImGui::Text(child.name.c_str());
        }
    }
}

void Editor::ContentDrawer::ScanDirectory(const std::filesystem::path& directory, FileNode& node)
{
    node.fullPath = directory.string();
    node.children.clear(); // Clear old children before rescanning

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        FileNode child;
        child.name = entry.path().filename().string();
        child.fullPath = entry.path().string();
        child.isDirectory = entry.is_directory();
        if (child.isDirectory) {
            ScanDirectory(entry.path(), child); // Recursively scan subdirectories
        }
        node.children.push_back(std::move(child));
    }
}
