#include "HRpch.h"
#include "SkeletonImporter.h"

#include <fstream>
#include "HRealEngine/Core/MeshLoader.h"
#include "HRealEngine/Project/Project.h"

namespace HRealEngine
{
    static bool ExtractCookedRelativePathSkeleton(const std::filesystem::path& hskeletonAbs, std::string& outCookedRel)
    {
        std::ifstream in(hskeletonAbs);
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

    Ref<Asset> SkeletonImporter::ImportSkeleton(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        std::string ext = metaData.FilePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext != ".hskeleton")
        {
            LOG_CORE_WARN("SkeletonImporter: Unsupported extension: {}", ext);
            return nullptr;
        }

        const auto assetsRoot = Project::GetActive()->GetAssetDirectory();

        const std::filesystem::path hskeletonAbs = assetsRoot / metaData.FilePath;

        std::string cookedRel;
        if (!ExtractCookedRelativePathSkeleton(hskeletonAbs, cookedRel))
        {
            LOG_CORE_ERROR("SkeletonImporter: Failed to parse Cooked from: {}", hskeletonAbs.string());
            return nullptr;
        }

        std::filesystem::path cookedAbs = assetsRoot / cookedRel;

        Ref<Skeleton> skel = MeshLoader::ReadHSkeletonBin(cookedAbs);
        if (!skel)
        {
            LOG_CORE_ERROR("SkeletonImporter: Failed to read cooked skeleton: {}", cookedAbs.string());
            return nullptr;
        }
        
        skel->Handle = assetHandle;

        LOG_CORE_INFO("Loaded skeleton: {} (Bones={})", metaData.FilePath.string(), (int)skel->Bones.size());
        return skel;
    }
}
