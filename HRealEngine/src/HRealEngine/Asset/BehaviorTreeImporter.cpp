#include "HRpch.h"
#include "BehaviorTreeImporter.h"

#include "BehaviorTreeThings/Core/BTSerializer.h"
#include "HRealEngine/BehaviorTreeThings/BehaviorTreeAsset.h"
#include "HRealEngine/Project/Project.h"

namespace HRealEngine
{
    Ref<Asset> BehaviorTreeImporter::ImportBehaviorTree(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        Ref<Asset> asset;
        auto path = Project::GetAssetDirectory() / metaData.FilePath;
        bool bExists = std::filesystem::exists(path);
        if (!bExists)
        {
            BTSerializer serializer;
            serializer.CreateBehaviorTreeFile(path.string());
            asset = CreateRef<BehaviorTreeAsset>(/*assetHandle, metaData*/);
            LOG_CORE_INFO("Behavior Tree file did not exist. Created new file at {}", path.string());
        }
        else
        {
            asset = CreateRef<BehaviorTreeAsset>(/*assetHandle, metaData*/);
            LOG_CORE_INFO("Imported Behavior Tree asset from {}", path.string());
        }
        return asset;
    }
}
