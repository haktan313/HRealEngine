
#include "ContentBrowserPanel.h"

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <imgui/imgui.h>

#include "HRealEngine/Asset/TextureImporter.h"
#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Core/ObjLoader.h"
#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Utils/PlatformUtils.h"

namespace HRealEngine
{
    static constexpr uint64_t kMaxImportFileBytes = 200ull * 1024ull * 1024ull;
    extern const std::filesystem::path g_AssetsDirectory = "assets"; 

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_BaseDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_BaseDirectory;
        
        /*m_FileIcon = Texture2D::Create("assets/textures/fileIcon.png");
        m_FolderIcon = Texture2D::Create("assets/textures/folderIcon.png");*/
        m_TreeNodes.push_back(TreeNode(".", 0)); // root node
        m_FileIcon = TextureImporter::LoadTexture("assets/textures/fileIcon.png");
        m_FolderIcon = TextureImporter::LoadTexture("assets/textures/folderIcon.png");
        RefreshAssetTree();
        m_Mode = Mode::FileSystem;
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

        const char* label = m_Mode == Mode::Asset ? "Asset" : "File";
        if (ImGui::Button(label))
        {
            m_Mode = m_Mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;
        }
        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
        {
            ImGui::SameLine();
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        if (m_Mode == Mode::Asset)
        {
            TreeNode* node = &m_TreeNodes[0];
            auto currentDir = std::filesystem::relative(m_CurrentDirectory, Project::GetAssetDirectory());
            for (const auto& part : currentDir)
            {
                if (node->Path == currentDir)
                    break;

                if (node->Children.find(part) != node->Children.end())
                {
                    node = &m_TreeNodes[node->Children[part]];
                }
                else
                {
                    HREALENGINE_CORE_DEBUGBREAK(false);
                }
            }
            for (const auto& [item, treeNodeIndex] : node->Children)
            {
                bool bisDir = std::filesystem::is_directory(Project::GetAssetDirectory() / item);
                std::string itemStr = item.generic_string();
                
                Ref<Texture2D> icon = bisDir ? m_FolderIcon : m_FileIcon;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton(itemStr.c_str(), (ImTextureID)icon->GetRendererID(), { sizeOfImages, sizeOfImages }, { 0, 1 }, { 1, 0 });

                if (ImGui::BeginPopupContextItem())
                {
                    /*if (ImGui::MenuItem("Import"))
                    {
                        auto relativePath = std::filesystem::relative(item, Project::GetAssetDirectory());
                        Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
                    }*/
                    if (ImGui::MenuItem("Delete"))
                    {
                    }
                    ImGui::EndPopup();
                }
                if (ImGui::BeginDragDropSource())
                {
                    AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle));
                    ImGui::EndDragDropSource();
                }

                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (bisDir)
                        m_CurrentDirectory /= item.filename();
                }
                ImGui::TextWrapped(itemStr.c_str());
                ImGui::NextColumn();
            }
        }
        else
        {
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
            {
                const auto& path = directoryEntry.path();
                std::string fileNameString = path.filename().string();

                Ref<Texture2D> icon = directoryEntry.is_directory() ? m_FolderIcon : m_FileIcon;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton(fileNameString.c_str(),(ImTextureID)(intptr_t)icon->GetRendererID(),{sizeOfImages, sizeOfImages},
                    {0,1}, {1,0});

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Import"))
                    {
                        auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
                        Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
                        RefreshAssetTree();
                    }
                    ImGui::EndPopup();
                }
                
                /*if (ImGui::BeginDragDropSource())
                {
                    //auto relativePath = std::filesystem::relative(directoryEntry.path(), g_AssetsDirectory);
                    std::filesystem::path relativePath(path);
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }*/
                
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    if (directoryEntry.is_directory())
                        m_CurrentDirectory /= directoryEntry.path().filename();
                
                ImGui::TextWrapped(fileNameString.c_str());
                ImGui::NextColumn();
            }
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

    void ContentBrowserPanel::RefreshAssetTree()
    {
        const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
        for (const auto& [handle, metadata] : assetRegistry)
        {
            uint32_t currentNodeIndex = 0;

            for (const auto& p : metadata.FilePath)
            {
                auto it = m_TreeNodes[currentNodeIndex].Children.find(p.generic_string());
                if (it != m_TreeNodes[currentNodeIndex].Children.end())
                {
                    currentNodeIndex = it->second;
                }
                else
                {
                    // add node
                    TreeNode newNode(p, handle);
                    newNode.Parent = currentNodeIndex;
                    m_TreeNodes.push_back(newNode);

                    m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
                    currentNodeIndex = m_TreeNodes.size() - 1;
                }

            }
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

        std::filesystem::path dstObj;
        std::filesystem::path lastCopiedTexAbs;
        std::vector<std::filesystem::path> texturePaths;
        CreateDirectoriesIfNotExists(srcObj, dstObj, lastCopiedTexAbs, texturePaths);   
        ImportDataFromOBJ(dstObj, lastCopiedTexAbs, texturePaths);
    }

    void ContentBrowserPanel::CreateDirectoriesIfNotExists(const std::filesystem::path& srcObj, std::filesystem::path& dstObj,
        std::filesystem::path& lastCopiedTexAbs,std::vector<std::filesystem::path>& texturePaths)
    {
        const std::string modelName = srcObj.stem().string();       
        std::filesystem::path importedDir = m_CurrentDirectory / "Imported" / modelName;
        std::filesystem::path sourceDir = importedDir / "Source";
        std::filesystem::path matsDir = importedDir / "Materials";
        std::filesystem::path texDir = importedDir / "Textures";
        
        std::filesystem::create_directories(sourceDir);
        std::filesystem::create_directories(matsDir);
        std::filesystem::create_directories(texDir);

        dstObj = sourceDir / srcObj.filename();
        dstObj = MakeUniquePath(dstObj);        
        if (!ObjLoader::CopyFileSafe(srcObj, dstObj))
        {
            m_LastError = "Failed to copy OBJ into assets Imported/Source.";
            m_OpenErrorPopup = true;
            return;
        }       

        std::vector<std::string> mtllibs = ObjLoader::ParseObjMtllibs(srcObj);
        if (!mtllibs.empty())
            LOG_CORE_INFO("OBJ mtllib count = {}", (int)mtllibs.size());
        
        for (const std::string& mtlRel : mtllibs)
        {
            std::filesystem::path srcMtlAbs = srcObj.parent_path() / mtlRel;
            if (!std::filesystem::exists(srcMtlAbs))
            {
                LOG_CORE_WARN("MTL missing next to OBJ: {}", srcMtlAbs.string());
                continue;
            }       

            std::filesystem::path dstMtlAbs = sourceDir / srcMtlAbs.filename();     
            std::vector<std::string> texRefs;
            if (!ObjLoader::RewriteMtlAndCollectTextures(srcMtlAbs, dstMtlAbs, texRefs))
            {
                LOG_CORE_WARN("Failed to rewrite/copy MTL: {}", srcMtlAbs.string());
                ObjLoader::CopyFileSafe(srcMtlAbs, dstMtlAbs);
                continue;
            }       

            //std::unordered_set<std::string> uniqueTex;
            //uniqueTex.reserve(texRefs.size());      
            for (auto& t : texRefs)
            {
                if (t.empty())
                    continue;
                //if (uniqueTex.find(t) != uniqueTex.end())
                //    continue;
                
               //uniqueTex.insert(t);        
                std::filesystem::path srcTexAbs = t;
                if (!srcTexAbs.is_absolute())
                    srcTexAbs = srcObj.parent_path() / srcTexAbs;       
                if (!std::filesystem::exists(srcTexAbs))
                {
                    LOG_CORE_WARN("Texture missing: {} (resolved to {})", t, srcTexAbs.string());
                    continue;
                }
                
                lastCopiedTexAbs = texDir / srcTexAbs.filename();        
                if (!ObjLoader::CopyFileSafe(srcTexAbs, lastCopiedTexAbs))
                    LOG_CORE_WARN("Failed to copy texture: {} -> {}", srcTexAbs.string(), lastCopiedTexAbs.string());
                else
                    LOG_CORE_INFO("Copied texture: {}", lastCopiedTexAbs.string());
                texturePaths.push_back(lastCopiedTexAbs);
            }       
            LOG_CORE_INFO("Copied+rewritten MTL: {}", dstMtlAbs.string());
        }    
    }

    void ContentBrowserPanel::ImportDataFromOBJ(const std::filesystem::path& dstObj,const std::filesystem::path& lastCopiedTexAbs,
        const std::vector<std::filesystem::path>& texturePaths)
    {
        std::vector<MeshVertex> verts;
        std::vector<uint32_t> inds;
        std::vector<HMeshBinSubmesh> submeshes;     
        if (!ObjLoader::LoadMeshFromFile(dstObj.string(), verts, inds, &submeshes))
        {
            LOG_CORE_INFO("OBJ load failed: {}", dstObj.string());
            return;
        }       

        std::filesystem::path cookedPath = g_AssetsDirectory / "cache";
        std::filesystem::create_directories(cookedPath);        
        cookedPath /= dstObj.stem();
        cookedPath += ".hmeshbin";      
        if (!ObjLoader::WriteHMeshBin(cookedPath, verts, inds, submeshes))
        {
            LOG_CORE_INFO("Cook write failed: {}", cookedPath.string());
            return;
        }       
        LOG_CORE_INFO("Cooked mesh: {} (V={}, I={})", cookedPath.string(), verts.size(), inds.size());      

        std::filesystem::path outMesh = m_CurrentDirectory / (dstObj.stem().string() + ".hmesh");
        outMesh = MakeUniquePath(outMesh);      
        auto sourceRel = std::filesystem::relative(dstObj, g_AssetsDirectory).generic_string();
        auto cookedRel = std::filesystem::relative(cookedPath, g_AssetsDirectory).generic_string();     

        auto materials = ObjLoader::ImportObjMaterialsToHMat(dstObj, m_CurrentDirectory, lastCopiedTexAbs, texturePaths);        
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
        out << "Materials:\n";
        for (const auto& m : materials)
            out << "  - " << m << "\n";
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
