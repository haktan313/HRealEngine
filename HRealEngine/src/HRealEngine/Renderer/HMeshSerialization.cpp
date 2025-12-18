#include "HRpch.h"
#include "HMeshSerialization.h"
#include <fstream>

namespace HRealEngine
{
    bool WriteHMeshBin(const std::filesystem::path& path, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
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

    bool ReadHMeshBin(const std::filesystem::path& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices)
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
}
