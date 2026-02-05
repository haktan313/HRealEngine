#pragma once
#include "Asset.h"

namespace HRealEngine
{
    class SkeletonImporter
    {
    public:
        static Ref<Asset> ImportSkeleton(AssetHandle assetHandle, const AssetMetadata& metaData);
    };
}
