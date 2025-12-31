#pragma once
#include "Asset.h"

namespace HRealEngine
{
    class MeshImporter
    {
    public:
        static Ref<Asset> ImportMesh(AssetHandle assetHandle, const AssetMetadata& metaData);
    };
}
