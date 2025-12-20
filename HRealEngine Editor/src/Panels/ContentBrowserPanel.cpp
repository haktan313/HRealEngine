
#include "ContentBrowserPanel.h"

#include <fstream>
#include <imgui/imgui.h>

#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Core/ObjLoader.h"
#include "HRealEngine/Utils/PlatformUtils.h"

namespace HRealEngine
{
    static constexpr uint64_t kMaxImportFileBytes = 20ull * 1024ull * 1024ull;
    extern const std::filesystem::path g_AssetsDirectory = "assets"; 

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(g_AssetsDirectory)
    {
        m_FileIcon = Texture2D::Create("assets/textures/fileIcon.png");
        m_FolderIcon = Texture2D::Create("assets/textures/folderIcon.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        if (ImGui::Button("Import OBJ"))
            ImportOBJ();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", m_CurrentDirectory.string().c_str());
        ImGui::Separator();
        
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
            ImGui::ImageButton(fileNameString.c_str(),(ImTextureID)(intptr_t)icon->GetRendererID(),{sizeOfImages, sizeOfImages},
                {0,1}, {1,0});

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

        if (m_OpenErrorPopup)
        {
            ImGui::OpenPopup("Import Error");
            m_OpenErrorPopup = false;
        }

        if (ImGui::BeginPopupModal("Import Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("%s", m_LastError.c_str());
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void ContentBrowserPanel::ImportOBJ()
    {
        std::string selected = FileDialogs::OpenFile("OBJ (*.obj)\0*.obj\0");
        if (selected.empty())
            return;
    
        std::filesystem::path srcObj = selected;
        uint64_t fileSize = 0;
        try
        {
            fileSize = std::filesystem::file_size(srcObj);
        }
        catch (...)
        {
            m_LastError = "Could not read OBJ file size.";
            m_OpenErrorPopup = true;
            return;
        }

        if (fileSize > kMaxImportFileBytes)
        {
            m_LastError = "OBJ file is too large for import budget.";
            m_OpenErrorPopup = true;
            return;
        }
        
        std::filesystem::path modelsDir = g_AssetsDirectory / "models";
        std::filesystem::create_directories(modelsDir);
    
        std::filesystem::path dstObj = modelsDir / srcObj.filename();
        dstObj = MakeUniquePath(dstObj);
        
        bool insideAssets = false;
        {
            auto absAssets = std::filesystem::absolute(g_AssetsDirectory);
            auto absSrc    = std::filesystem::absolute(srcObj);
            insideAssets = (absSrc.string().rfind(absAssets.string(), 0) == 0);
        }
    
        if (!insideAssets)
            std::filesystem::copy_file(srcObj, dstObj, std::filesystem::copy_options::overwrite_existing);
        else
            dstObj = std::filesystem::relative(srcObj, std::filesystem::current_path());
        
        std::vector<MeshVertex> verts;
        std::vector<uint32_t> inds;
    
        if (!ObjLoader::LoadMeshFromFile(dstObj.string(), verts, inds))
        {
            LOG_CORE_INFO("OBJ load failed: {}", dstObj.string());
            return;
        }
        
        std::filesystem::path cookedPath = g_AssetsDirectory / "cache";
        std::filesystem::create_directories(cookedPath);
    
        cookedPath /= dstObj.stem();
        cookedPath += ".hmeshbin";
    
        if (!ObjLoader::WriteHMeshBin(cookedPath, verts, inds))
        {
            LOG_CORE_INFO("Cook write failed: {}", cookedPath.string());
            return;
        }
    
        LOG_CORE_INFO("Cooked mesh: {} (V={}, I={})", cookedPath.string(), verts.size(), inds.size());
        
        std::filesystem::path outMesh = m_CurrentDirectory / (dstObj.stem().string() + ".hmesh");
        outMesh = MakeUniquePath(outMesh);
        
        auto sourceRel = std::filesystem::relative(dstObj, g_AssetsDirectory).generic_string();
        auto cookedRel = std::filesystem::relative(cookedPath, g_AssetsDirectory).generic_string();
    
        std::ofstream out(outMesh);
        out << "Type: StaticMesh\n";
        out << "Source: " << sourceRel << "\n";
        out << "Cooked: " << cookedRel << "\n";
        out << "Import:\n";
        out << "  Triangulate: true\n";
        out << "  GenSmoothNormals: true\n";
        out << "  CalcTangents: true\n";
        out << "  FlipUVs: false\n";
        out << "  Scale: 1.0\n";
        out.close();
    
        LOG_CORE_INFO("Created mesh asset: {}", outMesh.string());
    }

    std::filesystem::path ContentBrowserPanel::MakeUniquePath(const std::filesystem::path& p) const
    {
        if (!std::filesystem::exists(p))
            return p;

        const auto dir = p.parent_path();//assets/models
        const auto stem = p.stem().string();//model name
        const auto ext = p.extension().string();//.obj

        for (int i = 1; i < 10.000; i++)
        {
            std::filesystem::path candidate = dir / (stem + "_" + std::to_string(i) + ext);
            if (!std::filesystem::exists(candidate))
                return candidate;
        }
        return p;
    }
}
