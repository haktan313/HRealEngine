#include "HRpch.h"
#include "MeshAssetCache.h"
#include "HMeshAssetLoader.h"

namespace HRealEngine
{
    std::unordered_map<std::filesystem::path, Ref<MeshGPU>> MeshAssetCache::s_Cache;

    Ref<MeshGPU> MeshAssetCache::GetOrLoad(
        const std::filesystem::path& hmeshPath,
        const std::filesystem::path& assetsRoot,
        const Ref<Shader>& shader)
    {
        auto it = s_Cache.find(hmeshPath);
        if (it != s_Cache.end())
            return it->second;

        Ref<MeshGPU> mesh = LoadHMeshAsset(hmeshPath, assetsRoot, shader);
        if (mesh)
            s_Cache[hmeshPath] = mesh;

        return mesh;
    }

    void MeshAssetCache::Clear()
    {
        s_Cache.clear();
    }
}
