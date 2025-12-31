#include "HRpch.h"
#include "MeshImporter.h"

#include "AssetManager.h"
#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    Ref<Asset> MeshImporter::ImportMesh(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        LOG_CORE_INFO("Importing mesh asset from path: {}", metaData.FilePath.string());
        auto asset = ObjLoader::LoadHMeshAsset(metaData.FilePath, Project::GetActive()->GetAssetDirectory(), nullptr);
        return asset;
    }
}
