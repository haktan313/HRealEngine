
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

namespace HRealEngine
{
    extern const std::filesystem::path g_AssetsDirectory = "assets"; 

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(g_AssetsDirectory)
    {
        m_FileIcon = Texture2D::Create("assets/textures/fileIcon.png");
        m_FolderIcon = Texture2D::Create("assets/textures/folderIcon.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        
        static float sizeOfImages = 128.0f;
        static float distance = 16.0f;
        float cellSize = sizeOfImages + distance;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;
        
        ImGui::Columns(columnCount, 0, false);
        
        if (m_CurrentDirectory != std::filesystem::path(g_AssetsDirectory))
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            std::string path = directoryEntry.path().string();
            auto relativePath = std::filesystem::relative(directoryEntry.path(), g_AssetsDirectory);
            std::string fileNameString = relativePath.string();

            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_FolderIcon : m_FileIcon;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton(fileNameString.c_str(),(ImTextureID)(intptr_t)icon->GetRendererID(),{sizeOfImages, sizeOfImages}, {0,1}, {1,0});

            if (ImGui::BeginDragDropSource())
            {
                const wchar_t* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }
            
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= directoryEntry.path().filename();
            
            ImGui::TextWrapped(fileNameString.c_str());
            ImGui::NextColumn();
        }

        ImGui::Columns(1);

        ImGui::SliderFloat("Size", &sizeOfImages, 16, 512);
        ImGui::SliderFloat("Distance", &distance, 0, 32);
        
        ImGui::End();
    }
}
