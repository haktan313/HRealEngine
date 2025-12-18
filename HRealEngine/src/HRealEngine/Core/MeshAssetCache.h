#pragma once
#include <unordered_map>
#include <filesystem>

#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Mesh.h"
#include "HRealEngine/Renderer/Shader.h"

namespace HRealEngine
{
    class MeshAssetCache
    {
    public:
        static Ref<MeshGPU> GetOrLoad(
            const std::filesystem::path& hmeshPath,
            const std::filesystem::path& assetsRoot,
            const Ref<Shader>& shader);

        static void Clear();

    private:
        static std::unordered_map<std::filesystem::path, Ref<MeshGPU>> s_Cache;
    };
}
