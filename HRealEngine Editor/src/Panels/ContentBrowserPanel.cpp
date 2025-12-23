
#include "ContentBrowserPanel.h"

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <imgui/imgui.h>

#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Core/ObjLoader.h"
#include "HRealEngine/Utils/PlatformUtils.h"

namespace HRealEngine
{
    static std::string Trim(std::string s)
    {
        auto is_ws = [](unsigned char c)
        {
            return std::isspace(c) != 0;
        };
        
        while (!s.empty() && is_ws((unsigned char)s.front()))
            s.erase(s.begin());
        while (!s.empty() && is_ws((unsigned char)s.back()))
            s.pop_back();
        
        return s;
    }

    static std::string StripQuotes(std::string s)
    {
        s = Trim(std::move(s));
        if (s.size() >= 2)
        {
            if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))
                return s.substr(1, s.size() - 2);
        }
        return s;
    }
    
    static std::string RemainderAfterKeyword(const std::string& line, const std::string& keyword)
    {
        std::string rest = line.substr(keyword.size());
        return Trim(rest);
    }

    static bool StartsWith(const std::string& s, const char* prefix)
    {
        return s.rfind(prefix, 0) == 0;
    }

    static void NormalizeSlashes(std::string& s)
    {
        for (char& c : s) if (c == '\\') c = '/';
    }
    
    
    static std::vector<std::string> ParseObjMtllibs(const std::filesystem::path& objAbs)
    {
        std::vector<std::string> out;
        std::ifstream in(objAbs);
        if (!in)
            return out;

        std::string line;
        while (std::getline(in, line))
        {
            line = Trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (StartsWith(line, "mtllib"))
            {
                std::string rest = RemainderAfterKeyword(line, "mtllib");
                rest = StripQuotes(rest);
                NormalizeSlashes(rest);
                if (!rest.empty())
                    out.push_back(rest);
            }
        }
        return out;
    }
    
    static bool ExtractMtlMapPath(const std::string& line, std::string& outPath)
    {
        std::string s = Trim(line);
        if (s.empty() || s[0] == '#')
            return false;
        if (!StartsWith(s, "map_") && !StartsWith(s, "bump") && !StartsWith(s, "map_Bump"))
            return false;
        
        std::istringstream iss(s);
        std::string first;
        iss >> first;

        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok)
            tokens.push_back(tok);

        if (tokens.empty())
            return false;
        
        std::string candidate = tokens.back();
        candidate = StripQuotes(candidate);
        NormalizeSlashes(candidate);

        if (candidate.empty())
            return false;

        outPath = candidate;
        return true;
    }
    
    static bool RewriteMtlAndCollectTextures(const std::filesystem::path& srcMtlAbs, const std::filesystem::path& dstMtlAbs, std::vector<std::string>& outTextureRelOrAbs)
    {
        std::ifstream in(srcMtlAbs);
        if (!in)
            return false;

        std::vector<std::string> lines;
        lines.reserve(512);

        std::string line;
        while (std::getline(in, line))
        {
            std::string trimmed = Trim(line);

            std::string tex;
            if (ExtractMtlMapPath(trimmed, tex))
            {
                outTextureRelOrAbs.push_back(tex);
                
                std::filesystem::path p = tex;
                std::string newRef = std::string("../Textures/") + p.filename().generic_string();
                std::string original = line;
                size_t lastSpace = original.find_last_of(" \t");
                
                if (lastSpace != std::string::npos)
                {
                    original = original.substr(0, lastSpace + 1) + newRef;
                    line = original;
                }
                else
                {
                    line = newRef;
                }
            }
            lines.push_back(line);
        }

        std::ofstream out(dstMtlAbs);
        if (!out)
            return false;

        for (auto& l : lines)
            out << l << "\n";

        return true;
    }
    
    static bool CopyFileSafe(const std::filesystem::path& src, const std::filesystem::path& dst)
    {
        std::error_code ec;
        std::filesystem::create_directories(dst.parent_path(), ec);
        ec.clear();

        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
        return !ec;
    }
    
    static constexpr uint64_t kMaxImportFileBytes = 200ull * 1024ull * 1024ull;
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
        
        const std::string modelName = srcObj.stem().string();       
        std::filesystem::path importedDir = g_AssetsDirectory / "Imported" / modelName;
        std::filesystem::path sourceDir = importedDir / "Source";
        std::filesystem::path matsDir = importedDir / "Materials";
        std::filesystem::path texDir = importedDir / "Textures";
        
        std::filesystem::create_directories(sourceDir);
        std::filesystem::create_directories(matsDir);
        std::filesystem::create_directories(texDir);
        
        std::filesystem::path dstObj = sourceDir / srcObj.filename();
        dstObj = MakeUniquePath(dstObj);        
        if (!CopyFileSafe(srcObj, dstObj))
        {
            m_LastError = "Failed to copy OBJ into assets Imported/Source.";
            m_OpenErrorPopup = true;
            return;
        }       

        std::vector<std::string> mtllibs = ParseObjMtllibs(srcObj);
        std::vector<std::filesystem::path> texturePaths;
        if (!mtllibs.empty())
            LOG_CORE_INFO("OBJ mtllib count = {}", (int)mtllibs.size());

        std::filesystem::path lastCopiedTexAbs;
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
            if (!RewriteMtlAndCollectTextures(srcMtlAbs, dstMtlAbs, texRefs))
            {
                LOG_CORE_WARN("Failed to rewrite/copy MTL: {}", srcMtlAbs.string());
                CopyFileSafe(srcMtlAbs, dstMtlAbs);
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
                if (!CopyFileSafe(srcTexAbs, lastCopiedTexAbs))
                    LOG_CORE_WARN("Failed to copy texture: {} -> {}", srcTexAbs.string(), lastCopiedTexAbs.string());
                else
                    LOG_CORE_INFO("Copied texture: {}", lastCopiedTexAbs.string());
                texturePaths.push_back(lastCopiedTexAbs);
            }       
            LOG_CORE_INFO("Copied+rewritten MTL: {}", dstMtlAbs.string());
        }       

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

        auto materials = ObjLoader::ImportObjMaterialsToHMat(dstObj, g_AssetsDirectory, lastCopiedTexAbs, texturePaths);        
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
