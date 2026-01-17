#pragma once
#include "Asset.h"

namespace HRealEngine
{
    class BehaviorTreeImporter
    {
    public:
        static Ref<Asset> ImportBehaviorTree(AssetHandle assetHandle, const AssetMetadata& metaData);
    };
}
