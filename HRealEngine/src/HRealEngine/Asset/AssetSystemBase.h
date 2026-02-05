#pragma once
#include <filesystem>
#include <unordered_map>
#include <functional>
#include "HRealEngine/Core/Core.h"    

namespace HRealEngine
{
    class UUID;
    struct AssetMetadata;
    
    using AssetHandle = UUID;
    using AssetMap = std::unordered_map<AssetHandle, Ref<class Asset>>;
    using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;

    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
    
    enum class AssetType
    {
        None = 0,
        Texture,
        Scene,
        Mesh,
        Material,
        BehaviorTree,
        Skeleton
    };

    struct AssetMetadata
    {
        AssetType Type = AssetType::None;

        std::filesystem::path FilePath;

        operator bool() const { return Type != AssetType::None; }
    };
}
