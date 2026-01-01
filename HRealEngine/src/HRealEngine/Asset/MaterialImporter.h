#pragma once
#include "AssetSystemBase.h"


namespace HRealEngine
{
    class MaterialImporter
    {
    public:
        static Ref<Asset> ImportMaterial(AssetHandle assetHandle, const AssetMetadata& metaData);
    };
}
