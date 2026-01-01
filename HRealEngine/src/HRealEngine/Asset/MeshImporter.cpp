#include "HRpch.h"
#include "MeshImporter.h"

#include "AssetManager.h"
#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    Ref<Asset> MeshImporter::ImportMesh(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        std::string ext = metaData.FilePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        LOG_CORE_INFO("Importing mesh asset from path: {} (ext: {})", metaData.FilePath.string(), ext);

        Ref<Asset> asset = nullptr;

        if (ext == ".hmesh")
        {
            asset = ObjLoader::LoadHMeshAsset(metaData.FilePath, Project::GetActive()->GetAssetDirectory(), nullptr);
        }
        else if (ext == ".obj")
        {
            //Project::GetContentBrowserPanel()->ImportOBJ(Project::GetAssetFileSystemPath(metaData.FilePath));
            asset = ObjLoader::LoadHMeshAsset(metaData.FilePath, Project::GetActive()->GetAssetDirectory(), nullptr);
        }
        else
        {
            LOG_CORE_WARN("Unsupported mesh extension: {}", ext);
        }
        return asset;
    }
}
