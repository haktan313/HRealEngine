#pragma once
#include <filesystem>


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
        Material
    };

    struct AssetMetadata
    {
        AssetType Type = AssetType::None;

        std::filesystem::path FilePath;

        operator bool() const { return Type != AssetType::None; }
    };
}
