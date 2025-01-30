#include "ContentDrawer.h"

void Editor::ContentDrawer::Render()
{
    FileNode root = { .name= "Assets", .isDirectory= true, .children= std::vector<FileNode>() }; // Root directory name
    ScanDirectory("Assets", root);

    ImGui::Begin("Content Drawer");
    DrawFileTree(root);
    ImGui::End();
}

void Editor::ContentDrawer::DrawFileTree(const FileNode& node)
{
    if (node.isDirectory)
        {
        if (ImGui::TreeNode(node.name.c_str()))
            {
            for (const auto& child : node.children)
            {
                DrawFileTree(child); // Recursively draw children
            }
            ImGui::TreePop();
        }
    } else
    {
        ImGui::Text(node.name.c_str()); // Just display file names
    }
}

void Editor::ContentDrawer::ScanDirectory(const std::filesystem::path& directory, FileNode& node)
{
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        FileNode child;
        child.name = entry.path().filename().string();
        child.isDirectory = entry.is_directory();
        if (child.isDirectory)
            ScanDirectory(entry.path(), child); // Recurse for subdirectories
                
        node.children.push_back(std::move(child));
    }
}
