#include "HRpch.h"
#include "HMeshAssetLoader.h"

#include <fstream>
#include <string>

#include "HRealEngine/Renderer/HMeshSerialization.h"
#include "ObjLoader.h"

namespace HRealEngine
{
    static bool ExtractCookedRelativePath(const std::filesystem::path& hmeshPath, std::string& outCookedRel)
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

    Ref<MeshGPU> LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader)
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

        LOG_CORE_INFO("Loaded cooked mesh: {} (V={}, I={})",
            cookedAbs.string(), verts.size(), inds.size());

        return ObjLoader::BuildStaticMeshGPU(verts, inds, shader);
    }

}
