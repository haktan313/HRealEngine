#pragma once
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "HRealEngine/Asset/Asset.h"
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Shader.h"
#include "HRealEngine/Renderer/VertexArray.h"

namespace HRealEngine
{
    struct HMeshBinSubmesh
    {
        uint32_t IndexOffset = 0;
        uint32_t IndexCount = 0;
        uint32_t MaterialIndex = 0; // from aiMesh->mMaterialIndex
    };
    struct MeshVertex
    {
        glm::vec3 Position{0.0f};
        glm::vec3 Normal {0.0f, 1.0f, 0.0f};
        glm::vec2 UV {0.0f};
        glm::vec3 Tangent {1.0f, 0.0f, 0.0f};
        glm::vec3 Color {1.0f};
    };
    struct HMeshBinHeader
    {
        uint32_t Magic = 0x48534D48;
        uint32_t Version = 2;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
        uint32_t SubmeshCount = 0;
    };
    class MeshGPU : public Asset
    {
    public:
        virtual ~MeshGPU() = default;

        static AssetType GetStaticType() { return AssetType::Mesh; }
        AssetType GetType() const override { return GetStaticType(); }

        Ref<VertexArray> VAO;
        Ref<Shader> Shader;
        uint32_t IndexCount = 0;
        std::vector<HMeshBinSubmesh> Submeshes;
        //std::vector<std::string> MaterialPaths;
        std::vector<AssetHandle> MaterialHandles;
    };
    class ObjLoader
    {
    public:
        static bool LoadMeshFromFile(const std::string& path, std::vector<MeshVertex>& outVertices,
            std::vector<uint32_t>& outIndices, std::vector<HMeshBinSubmesh>* outSubmeshes);
        static std::vector<std::string> ImportObjMaterialsToHMat(const std::filesystem::path& objPathInAssets, const std::filesystem::path& assetsRoot, const std::filesystem::path& lastCopiedTexAbs, const std::vector<std::filesystem::path>& texturePaths);
        static Ref<MeshGPU> LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader);
        static bool WriteHMeshBin(const std::filesystem::path& path,
            const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<HMeshBinSubmesh>& submeshes);
        static bool ReadHMeshBin(const std::filesystem::path& path,
            std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices, std::vector<HMeshBinSubmesh>* outSubmeshes = nullptr);
        static Ref<MeshGPU> GetOrLoad(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader);
        static bool ParseHMeshMaterials(const std::filesystem::path& hmeshAbs, std::vector<std::string>& outMaterials);
        static bool ParseHMeshMaterialHandles(const std::filesystem::path& hmeshAbs, std::vector<AssetHandle>& outHandles);
        static bool ExtractCookedRelativePath(const std::filesystem::path& hmeshPath, std::string& outCookedRel);
        static bool TryResolveTexturePath(const std::filesystem::path& objAbs, const std::string& texRelOrAbs,std::filesystem::path& outAbs);
        static void Clear();
        static std::string SanitizeName(std::string s);
        
        static std::string Trim(std::string s);
        static std::string StripQuotes(std::string s);
        static std::string RemainderAfterKeyword(const std::string& line, const std::string& keyword);
        static bool StartsWith(const std::string& s, const char* prefix);
        static void NormalizeSlashes(std::string& s);
        static std::vector<std::string> ParseObjMtllibs(const std::filesystem::path& objAbs);
        static bool ExtractMtlMapPath(const std::string& line, std::string& outPath);
        static bool RewriteMtlAndCollectTextures(const std::filesystem::path& srcMtlAbs, const std::filesystem::path& dstMtlAbs, std::vector<std::string>& outTextureRelOrAbs);
        static bool CopyFileSafe(const std::filesystem::path& src, const std::filesystem::path& dst);
    private:
        static std::unordered_map<std::filesystem::path, Ref<MeshGPU>> s_Cache;
    };
}
