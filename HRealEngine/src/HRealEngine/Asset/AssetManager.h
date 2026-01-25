#pragma once
#include "Asset.h"
#include "HRealEngine/Core/UUID.h"
#include "HRealEngine/Project/Project.h"

namespace HRealEngine
{
    class AssetManager
    {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle)
        {
            Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(assetHandle);
            return std::static_pointer_cast<T>(asset);
        }
        
        static bool IsAssetHandleValid(AssetHandle handle)
        {
            return Project::GetActive()->GetAssetManager()->IsAssetHandleValid(handle);
        }

        static bool IsAssetLoaded(AssetHandle handle)
        {
            return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);
        }

        static AssetType GetAssetType(AssetHandle handle)
        {
            return Project::GetActive()->GetAssetManager()->GetAssetType(handle);
        }

        static std::vector<AssetHandle> GetAllAssetsOfType(AssetType type)
        {
            std::vector<AssetHandle> handles;
            const AssetRegistry& registry = Project::GetActive()->GetAssetManager()->GetAssetRegistry();
            for (const auto& [handle, metadata] : registry)
            {
                if (metadata.Type == type)
                    handles.push_back(handle);
            }
            return handles;
        }
    };
}
