#pragma once
#include "AssetSystemBase.h"
#include "HRealEngine/Core/UUID.h"

namespace HRealEngine
{
    std::string_view AssetTypeToString(AssetType type);
    AssetType AssetTypeFromString(std::string_view assetType);
    
    class Asset
    {
    public:
        AssetHandle Handle;
        
        virtual AssetType GetType() const = 0;
        
    };
}
