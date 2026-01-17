#include "HRpch.h"
#include "AssetImporter.h"

#include "BehaviorTreeImporter.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "SceneImporter.h"
#include "TextureImporter.h"

namespace HRealEngine
{
    static std::map<AssetType, AssetImportFunction> s_AssetImporters = {
        { AssetType::Texture, TextureImporter::ImportTexture },
        { AssetType::Scene, SceneImporter::ImportScene },
        { AssetType::Mesh, MeshImporter::ImportMesh },
        { AssetType::Material, MaterialImporter::ImportMaterial },
        {AssetType::BehaviorTree, BehaviorTreeImporter::ImportBehaviorTree }
    };
    
    Ref<Asset> AssetImporter::ImportAsset(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        if (s_AssetImporters.find(metaData.Type) == s_AssetImporters.end())
        {
            LOG_CORE_ERROR("No importer registered for asset type {}", static_cast<int>(metaData.Type));
            return nullptr;
        }
        return s_AssetImporters[metaData.Type](assetHandle, metaData);
    }
}
