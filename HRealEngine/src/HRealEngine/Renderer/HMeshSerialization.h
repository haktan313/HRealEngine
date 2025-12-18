#pragma once
#include <filesystem>
#include <vector>
#include <cstdint>

#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    struct HMeshBinHeader
    {
        uint32_t Magic = 0x48534D48;
        uint32_t Version = 1;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
    };

    bool WriteHMeshBin(const std::filesystem::path& path, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);
    bool ReadHMeshBin(const std::filesystem::path& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices);
}
