#pragma once
#include "Asset.h"

namespace HRealEngine
{
    class AssetManagerBase
    {
    public:
        virtual ~AssetManagerBase() = default;
        virtual Ref<Asset> GetAsset(AssetHandle assetHandle) = 0;
        
        virtual bool IsAssetHandleValid(AssetHandle assetHandle) const = 0;
        virtual bool IsAssetLoaded(AssetHandle assetHandle) const = 0;
        virtual AssetType GetAssetType(AssetHandle assetHandle) const = 0;
    };
}
