#include "HRpch.h"
#include "ObjLoader.h"

#include <filesystem>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Renderer/Renderer3D.h"

namespace HRealEngine
{
    std::unordered_map<std::filesystem::path, Ref<MeshGPU>> ObjLoader::s_Cache;
    
    glm::vec3 ToVec3(const aiVector3D& v) { return { v.x, v.y, v.z }; }
    glm::vec2 ToVec2(const aiVector3D& v) { return { v.x, v.y }; }

    bool ObjLoader::LoadMeshFromFile(const std::string& path,
        std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices, std::vector<HMeshBinSubmesh>* outSubmeshes)
    {
        Assimp::Importer importer;

        const unsigned flags =
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ImproveCacheLocality;

        const aiScene* scene = importer.ReadFile(path.c_str(), flags);

        if (!scene || !scene->HasMeshes())
        {
            std::cerr << "Assimp failed: " << importer.GetErrorString() << "\n";
            return false;
        }

        outVertices.clear();
        outIndices.clear();
        outSubmeshes->clear();

        for (unsigned m = 0; m < scene->mNumMeshes; ++m)
        {
            aiMesh* mesh = scene->mMeshes[m];
            
            const uint32_t baseVertex = (uint32_t)outVertices.size();

            const uint32_t indexStart = (uint32_t)outIndices.size();
            HMeshBinSubmesh submesh{};
            submesh.IndexOffset = indexStart;
            submesh.MaterialIndex = mesh->mMaterialIndex;

            const bool hasNormals = mesh->HasNormals();
            const bool hasUV0 = mesh->HasTextureCoords(0);
            const bool hasTangents = mesh->HasTangentsAndBitangents();
            const bool hasColors0 = mesh->HasVertexColors(0);

            outVertices.reserve(outVertices.size() + mesh->mNumVertices);

            for (unsigned i = 0; i < mesh->mNumVertices; ++i)
            {
                MeshVertex v{};
                v.Position = ToVec3(mesh->mVertices[i]);
                v.Normal = hasNormals ? ToVec3(mesh->mNormals[i]) : glm::vec3(0,1,0);
                v.UV = hasUV0 ? ToVec2(mesh->mTextureCoords[0][i]) : glm::vec2(0);
                if (hasTangents)
                    v.Tangent = ToVec3(mesh->mTangents[i]);
                v.Color = glm::vec3(1.0f);
                if (hasColors0)
                {
                    const aiColor4D c = mesh->mColors[0][i];
                    v.Color = glm::vec3(c.r, c.g, c.b);
                }
                outVertices.push_back(v);
            }

            for (unsigned f = 0; f < mesh->mNumFaces; ++f)
            {
                const aiFace& face = mesh->mFaces[f];
                if (face.mNumIndices != 3)
                    continue;

                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[0]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[1]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[2]);
            }

            const uint32_t indexEnd = (uint32_t)outIndices.size();
            submesh.IndexCount = indexEnd - indexStart;

            if (outSubmeshes && submesh.IndexCount > 0)
                outSubmeshes->push_back(submesh);
        }

        std::cout << "Loaded: " << path << " | V: " << outVertices.size() << " | I: " << outIndices.size() << "\n";
        return true;
    }
    
    std::vector<std::string> ObjLoader::ImportObjMaterialsToHMat(const std::filesystem::path& objPathInAssets,
        const std::filesystem::path& assetsRoot, const std::filesystem::path& lastCopiedTexAbs, const std::vector<std::filesystem::path>& texturePaths)
    {
        const std::filesystem::path objAbs = std::filesystem::absolute(objPathInAssets);

        Assimp::Importer importer;
        const unsigned flags =
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace;

        const aiScene* scene = importer.ReadFile(objAbs.string(), flags);
        if (!scene)
        {
            LOG_CORE_ERROR("Assimp material import failed: {}", importer.GetErrorString());
            return {};
        }

        std::unordered_set<unsigned> used;
        used.reserve(scene->mNumMeshes);

        for (unsigned m = 0; m < scene->mNumMeshes; ++m)
        {
            aiMesh* mesh = scene->mMeshes[m];
            if (!mesh)
                continue;
            used.insert(mesh->mMaterialIndex);
        }

        LOG_CORE_INFO("Assimp: Meshes={}, Materials={}, UsedMaterials={}",
            scene->mNumMeshes, scene->mNumMaterials, used.size());
        
        std::vector<std::string> materialRelPaths(scene->mNumMaterials, "null");

        const std::string modelName = objPathInAssets.stem().string();

        std::filesystem::path importedDir = assetsRoot / "Imported" / modelName;
        std::filesystem::path matsDir = importedDir / "Materials";
        std::filesystem::path texDir = importedDir / "Textures";

        std::filesystem::create_directories(matsDir);
        std::filesystem::create_directories(texDir);

        for (unsigned i = 0; i < scene->mNumMaterials; ++i)
        {
            if (used.find(i) == used.end())
                continue;

            aiMaterial* mat = scene->mMaterials[i];
            if (!mat)
                continue;

            aiString aiName;
            mat->Get(AI_MATKEY_NAME, aiName);
            std::string matName = aiName.length > 0 ? aiName.C_Str() : ("mat_" + std::to_string(i));
            matName = SanitizeName(matName);
            
            const std::string fileName = modelName + "_" + matName + ".hmat";

            aiColor3D kd(1.0f, 1.0f, 1.0f);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
            
            std::string albedoTexRel = "null";
            if (!texturePaths.empty() && i - 1 < texturePaths.size())
                albedoTexRel = std::filesystem::relative(texturePaths[i - 1], assetsRoot).generic_string();
            
            std::filesystem::path hmatAbs = matsDir / fileName;
            
            AssetHandle albedoHandle = 0;
            if (albedoTexRel != "null" && !albedoTexRel.empty())
            {
                auto editorAssetManager = Project::GetActive()->GetEditorAssetManager();
                albedoHandle = editorAssetManager->GetHandleFromPath(std::filesystem::path(albedoTexRel));
            }

            auto textureDist = lastCopiedTexAbs;
            LOG_CORE_INFO("  (Source Texture: {})", textureDist.string());
            std::ofstream hm(hmatAbs);
            hm << "Type: Material\n";
            hm << "Shader: shaders/StaticMesh.glsl\n";
            hm << "BaseColor: [" << kd.r << ", " << kd.g << ", " << kd.b << "]\n";
            hm << "AlbedoTextureHandle: " << (uint64_t)albedoHandle << "\n";
            //hm << "AlbedoTexture: " << albedoTexRel << "\n";
            
            hm << "SpecularTextureHandle: " << (uint64_t)0 << "\n";
            hm << "NormalTextureHandle: "   << (uint64_t)0 << "\n";
            hm << "Shininess: " << 32.0f << "\n";
            hm.close();

            materialRelPaths[i] = std::filesystem::relative(hmatAbs, assetsRoot).generic_string();

            LOG_CORE_INFO("Created HMAT (matIndex={}): {} (AlbedoTexture={})",
                i, materialRelPaths[i], albedoTexRel);
            Project::GetActive()->GetEditorAssetManager()->ImportAsset(hmatAbs);
        }

        return materialRelPaths;
    }

    bool ObjLoader::WriteHMeshBin(const std::filesystem::path& path, const std::vector<MeshVertex>& vertices,
        const std::vector<uint32_t>& indices, const std::vector<HMeshBinSubmesh>& submeshes)
    {
        std::filesystem::create_directories(path.parent_path());

        std::ofstream out(path, std::ios::binary);
        if (!out)
            return false;

        HMeshBinHeader header;
        header.Version = 2;
        header.VertexCount = (uint32_t)vertices.size();
        header.IndexCount  = (uint32_t)indices.size();
        header.SubmeshCount = (uint32_t)submeshes.size();

        out.write((const char*)&header, sizeof(header));
        if (!submeshes.empty())
            out.write((const char*)submeshes.data(), sizeof(HMeshBinSubmesh) * submeshes.size());
        out.write((const char*)vertices.data(), sizeof(MeshVertex) * vertices.size());
        out.write((const char*)indices.data(),  sizeof(uint32_t) * indices.size());
        return true;
    }

    bool ObjLoader::ReadHMeshBin(const std::filesystem::path& path, std::vector<MeshVertex>& outVertices,
        std::vector<uint32_t>& outIndices, std::vector<HMeshBinSubmesh>* outSubmeshes)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
        {
            LOG_CORE_ERROR("Failed to open HMeshBin: {}", path.string());
            return false;
        }

        HMeshBinHeader header;
        in.read((char*)&header, sizeof(header));

        if (header.Magic != 0x48534D48 || header.Version != 2)
            return false;

        std::vector<HMeshBinSubmesh> localSubmeshes;
        localSubmeshes.resize(header.SubmeshCount);
        if (header.SubmeshCount > 0)
            in.read((char*)localSubmeshes.data(), sizeof(HMeshBinSubmesh) * localSubmeshes.size());

        outVertices.resize(header.VertexCount);
        outIndices.resize(header.IndexCount);

        in.read((char*)outVertices.data(), sizeof(MeshVertex) * outVertices.size());
        in.read((char*)outIndices.data(),  sizeof(uint32_t) * outIndices.size());
        if (outSubmeshes)
            *outSubmeshes = std::move(localSubmeshes);
        return true;
    }

    Ref<MeshGPU> ObjLoader::GetOrLoad(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot,
        const Ref<Shader>& shader)
    {
        auto it = s_Cache.find(hmeshPath);
        if (it != s_Cache.end())
            return it->second;

        auto assetFolder = Project::GetActive()->GetAssetDirectory();
        Ref<MeshGPU> mesh = LoadHMeshAsset(hmeshPath, assetFolder, shader);
        if (mesh)
            s_Cache[hmeshPath] = mesh;

        return mesh;
    }

    bool ObjLoader::ParseHMeshMaterials(const std::filesystem::path& hmeshAbs, std::vector<std::string>& outMaterials)
    {
        std::ifstream in(hmeshAbs);
        if (!in)
            return false;

        outMaterials.clear();

        std::string line;
        bool inMaterials = false;

        while (std::getline(in, line))
        {
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
                continue;
            std::string s = line.substr(start);

            if (!inMaterials)
            {
                if (s == "Materials:" || s.rfind("Materials:", 0) == 0)
                {
                    inMaterials = true;
                }
                continue;
            }
            
            if (start == 0 && s.find(':') != std::string::npos && s.rfind("-", 0) != 0)
                break;
            
            if (s.rfind("-", 0) == 0)
            {
                std::string val = s.substr(1);

                size_t vstart = val.find_first_not_of(" \t");
                if (vstart == std::string::npos)
                    val = "";
                else
                    val = val.substr(vstart);

                if (val.empty())
                    val = "null";
                outMaterials.push_back(val);
            }
        }

        return true;
    }

        bool ObjLoader::ParseHMeshMaterialHandles(const std::filesystem::path& hmeshAbs, /*std::vector<std::string>& outMaterials*/std::vector<AssetHandle>& outHandles)
    {
        std::ifstream in(hmeshAbs);
        if (!in)
            return false;

        //outMaterials.clear();

        std::string line;
        //bool inMaterials = false;
        bool inSection = false;

        while (std::getline(in, line))
        {
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
                continue;
            std::string s = line.substr(start);

            if (!inSection)
            {
                if (s == "MaterialHandles:" || s.rfind("MaterialHandles:", 0) == 0)
                    inSection = true;
                continue;
            }
            
            /*if (!inMaterials)
            {
                if (s == "Materials:" || s.rfind("Materials:", 0) == 0)
                {
                    inMaterials = true;
                }
                continue;
            }*/
            
            if (start == 0 && s.find(':') != std::string::npos && s.rfind("-", 0) != 0)
                break;
            
            if (s.rfind("-", 0) == 0)
            {
                /*std::string val = s.substr(1);

                size_t vstart = val.find_first_not_of(" \t");
                if (vstart == std::string::npos)
                    val = "";
                else
                    val = val.substr(vstart);

                if (val.empty())
                    val = "null";
                outMaterials.push_back(val);*/
                std::string val = s.substr(1);
                size_t vstart = val.find_first_not_of(" \t");
                val = (vstart == std::string::npos) ? "" : val.substr(vstart);

                uint64_t id = 0;
                try { id = std::stoull(val); } catch (...) { id = 0; }
                outHandles.push_back((AssetHandle)id);
            }
        }

        return true;
    }

    Ref<MeshGPU> ObjLoader::LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot,
        const Ref<Shader>& shader)
    {
        std::filesystem::path hmeshAbs = assetsRoot / hmeshPath;

        std::string cookedRel;
        if (!ExtractCookedRelativePath(hmeshAbs, cookedRel))
        {
            LOG_CORE_ERROR("Failed to parse Cooked path from: {}", hmeshAbs.string());
            return nullptr;
        }

        std::filesystem::path cookedAbs = assetsRoot / cookedRel;

        std::vector<MeshVertex> verts;
        std::vector<uint32_t> inds;
        std::vector<HMeshBinSubmesh> submeshes;
        
        if (!ReadHMeshBin(cookedAbs, verts, inds, &submeshes))
        {
            LOG_CORE_ERROR("Failed to read cooked mesh: {}", cookedAbs.string());
            return nullptr;
        }

        LOG_CORE_INFO("Loaded cooked mesh: {} (V={}, I={})", cookedAbs.string(), verts.size(), inds.size());

        Ref<MeshGPU> mesh = Renderer3D::BuildStaticMeshGPU(verts, inds, shader);

        std::vector<AssetHandle> handles;
        if (ParseHMeshMaterialHandles(hmeshAbs, handles))
        {
            mesh->MaterialHandles = std::move(handles);
        }
        else
        {
            std::vector<std::string> mats;
            ParseHMeshMaterials(hmeshAbs, mats);

            mesh->MaterialHandles.clear();
            mesh->MaterialHandles.reserve(mats.size());

            auto eam = Project::GetActive()->GetEditorAssetManager();
            for (auto& m : mats)
            {
                if (m.empty() || m == "null")
                {
                    mesh->MaterialHandles.push_back(0);
                    continue;
                }

                std::filesystem::path relHmat = m;
                AssetHandle h = eam->GetHandleFromPath(relHmat);
                mesh->MaterialHandles.push_back(h);

                if (h == 0)
                    LOG_CORE_WARN("HMAT not found in AssetRegistry: {}", relHmat.string());
            }
        }
        
        //mesh->MaterialPaths = std::move(mats);

        if (mesh)
            mesh->Submeshes = std::move(submeshes);

        return mesh;
    }

    bool ObjLoader::ExtractCookedRelativePath(const std::filesystem::path& hmeshPath, std::string& outCookedRel)
    {
        std::ifstream in(hmeshPath);
        if (!in)
            return false;

        std::string line;
        while (std::getline(in, line))
        {
            const std::string key = "Cooked:";
            if (line.rfind(key, 0) == 0)
            {
                std::string value = line.substr(key.size());
                size_t first = value.find_first_not_of(" \t");
                if (first != std::string::npos)
                    value = value.substr(first);

                outCookedRel = value;
                return !outCookedRel.empty();
            }
        }
        return false;
    }

    bool ObjLoader::TryResolveTexturePath(const std::filesystem::path& objAbs, const std::string& texRelOrAbs,
        std::filesystem::path& outAbs)
    {
        std::filesystem::path p = texRelOrAbs;
        
        std::string fixed = texRelOrAbs;
        for (char& c : fixed) if (c == '\\') c = '/';
        p = fixed;

        if (p.is_absolute())
        {
            if (std::filesystem::exists(p))
            {
                outAbs = p;
                return true;
            }
            return false;
        }

        std::filesystem::path candidate = objAbs.parent_path() / p;
        if (std::filesystem::exists(candidate))
        {
            outAbs = candidate;
            return true;
        }

        return false;
    }

    void ObjLoader::Clear()
    {
        s_Cache.clear();
    }

    std::string ObjLoader::SanitizeName(std::string s)
    {
        for (char& c : s)
        {
            const bool ok =
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-' || c == '.';
            if (!ok)
                c = '_';
        }
        if (s.empty()) s = "Material";
        return s;
    }

    

    std::string ObjLoader::Trim(std::string s)
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

    std::string ObjLoader::StripQuotes(std::string s)
    {
        s = Trim(std::move(s));
        if (s.size() >= 2)
        {
            if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))
                return s.substr(1, s.size() - 2);
        }
        return s;
    }
    
    std::string ObjLoader::RemainderAfterKeyword(const std::string& line, const std::string& keyword)
    {
        std::string rest = line.substr(keyword.size());
        return Trim(rest);
    }

    bool ObjLoader::StartsWith(const std::string& s, const char* prefix)
    {
        return s.rfind(prefix, 0) == 0;
    }

    void ObjLoader::NormalizeSlashes(std::string& s)
    {
        for (char& c : s) if (c == '\\') c = '/';
    }
    
    std::vector<std::string> ObjLoader::ParseObjMtllibs(const std::filesystem::path& objAbs)
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
    
    bool ObjLoader::ExtractMtlMapPath(const std::string& line, std::string& outPath)
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
    
    bool ObjLoader::RewriteMtlAndCollectTextures(const std::filesystem::path& srcMtlAbs, const std::filesystem::path& dstMtlAbs, std::vector<std::string>& outTextureRelOrAbs)
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
    
    bool ObjLoader::CopyFileSafe(const std::filesystem::path& src, const std::filesystem::path& dst)
    {
        std::error_code ec;
        std::filesystem::create_directories(dst.parent_path(), ec);
        ec.clear();

        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
        return !ec;
    }
}
