#pragma once
#include "Asset.h"
#include "HRealEngine/Core/UUID.h"

namespace HRealEngine
{
    class AssetImporter
    {
    public:
        static Ref<Asset> ImportAsset(AssetHandle assetHandle, const AssetMetadata& metaData);
    };
}
