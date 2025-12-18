#include "HRpch.h"
#include "ObjLoader.h"

#include <filesystem>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "HRealEngine/Renderer/Buffers.h"
#include "HRealEngine/Renderer/Renderer3D.h"

namespace HRealEngine
{
    std::unordered_map<std::filesystem::path, Ref<MeshGPU>> ObjLoader::s_Cache;
    
    glm::vec3 ToVec3(const aiVector3D& v) { return { v.x, v.y, v.z }; }
    glm::vec2 ToVec2(const aiVector3D& v) { return { v.x, v.y }; }

    bool ObjLoader::LoadMeshFromFile(const std::string& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices)
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

        for (unsigned m = 0; m < scene->mNumMeshes; ++m)
        {
            aiMesh* mesh = scene->mMeshes[m];
            const uint32_t baseVertex = (uint32_t)outVertices.size();

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
                if (face.mNumIndices != 3) continue;

                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[0]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[1]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[2]);
            }
        }

        std::cout << "Loaded: " << path << " | V: " << outVertices.size() << " | I: " << outIndices.size() << "\n";
        return true;
    }

    bool ObjLoader::WriteHMeshBin(const std::filesystem::path& path, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
    {
        std::filesystem::create_directories(path.parent_path());

        std::ofstream out(path, std::ios::binary);
        if (!out)
            return false;

        HMeshBinHeader header;
        header.VertexCount = (uint32_t)vertices.size();
        header.IndexCount  = (uint32_t)indices.size();

        out.write((const char*)&header, sizeof(header));
        out.write((const char*)vertices.data(), sizeof(MeshVertex) * vertices.size());
        out.write((const char*)indices.data(),  sizeof(uint32_t) * indices.size());
        return true;
    }

    bool ObjLoader::ReadHMeshBin(const std::filesystem::path& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return false;

        HMeshBinHeader header;
        in.read((char*)&header, sizeof(header));

        if (header.Magic != 0x48534D48 || header.Version != 1)
            return false;

        outVertices.resize(header.VertexCount);
        outIndices.resize(header.IndexCount);

        in.read((char*)outVertices.data(), sizeof(MeshVertex) * outVertices.size());
        in.read((char*)outIndices.data(),  sizeof(uint32_t) * outIndices.size());
        return true;
    }

    Ref<MeshGPU> ObjLoader::GetOrLoad(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader)
    {
        auto it = s_Cache.find(hmeshPath);
        if (it != s_Cache.end())
            return it->second;

        Ref<MeshGPU> mesh = LoadHMeshAsset(hmeshPath, assetsRoot, shader);
        if (mesh)
            s_Cache[hmeshPath] = mesh;

        return mesh;
    }

    Ref<MeshGPU> ObjLoader::LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader)
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
        if (!ReadHMeshBin(cookedAbs, verts, inds))
        {
            LOG_CORE_ERROR("Failed to read cooked mesh: {}", cookedAbs.string());
            return nullptr;
        }

        LOG_CORE_INFO("Loaded cooked mesh: {} (V={}, I={})", cookedAbs.string(), verts.size(), inds.size());

        return Renderer3D::BuildStaticMeshGPU(verts, inds, shader);
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

    void ObjLoader::Clear()
    {
        s_Cache.clear();
    }
}
