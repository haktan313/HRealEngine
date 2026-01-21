#include "HRpch.h"
#include "MaterialImporter.h"

#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Renderer/Material.h"

namespace HRealEngine
{
    Ref<Asset> MaterialImporter::ImportMaterial(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        auto mat = MaterialLibrary::GetOrLoad(metaData.FilePath, Project::GetActive()->GetAssetDirectory());
        if (mat)
            mat->Handle = assetHandle;
        return mat;
    }
}
