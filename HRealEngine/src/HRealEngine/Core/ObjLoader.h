#pragma once

#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Shader.h"
#include "HRealEngine/Renderer/VertexArray.h"

namespace HRealEngine
{
    struct MeshVertex
    {
        glm::vec3 Position{0.0f};
        glm::vec3 Normal  {0.0f, 1.0f, 0.0f};
        glm::vec2 UV      {0.0f};
        glm::vec3 Tangent {1.0f, 0.0f, 0.0f};
        glm::vec3 Color   {1.0f};
    };
    struct MeshGPU
    {
        Ref<VertexArray> VAO;
        Ref<Shader> Shader;
        uint32_t IndexCount = 0;
    };
    struct HMeshBinHeader
    {
        uint32_t Magic = 0x48534D48;
        uint32_t Version = 1;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
    };
    class ObjLoader
    {
    public:
        static bool LoadMeshFromFile(const std::string& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices);

        static bool WriteHMeshBin(const std::filesystem::path& path, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);
        static bool ReadHMeshBin(const std::filesystem::path& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices);

        static Ref<MeshGPU> GetOrLoad(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader);
        static Ref<MeshGPU> LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader);
        static bool ExtractCookedRelativePath(const std::filesystem::path& hmeshPath, std::string& outCookedRel);
        static void Clear();
    private:
        static std::unordered_map<std::filesystem::path, Ref<MeshGPU>> s_Cache;
    };
}
