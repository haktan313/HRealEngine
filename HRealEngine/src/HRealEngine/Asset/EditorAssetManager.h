#pragma once
#include "AssetManagerBase.h"

namespace HRealEngine
{
    class EditorAssetManager : public AssetManagerBase
    {
    public:
        virtual Ref<Asset> GetAsset(AssetHandle assetHandle) override;

        virtual bool IsAssetLoaded(AssetHandle assetHandle) const override { return m_LoadedAssets.find(assetHandle) != m_LoadedAssets.end(); }
        virtual bool IsAssetHandleValid(AssetHandle assetHandle) const override { return assetHandle != 0 && m_AssetRegistry.find(assetHandle) != m_AssetRegistry.end(); }
        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }
        virtual AssetType GetAssetType(AssetHandle assetHandle) const override;
        AssetHandle GetHandleFromPath(const std::filesystem::path& relPath) const;

        Ref<Asset> ReloadAsset(AssetHandle handle);

        void ImportAsset(const std::filesystem::path& filePath);
        void SerializeAssetRegistry();
        bool DeserializeAssetRegistry();

        const AssetMetadata& GetAssetMetadata(AssetHandle assetHandle) const;
        const std::filesystem::path& GetAssetFilePath(AssetHandle assetHandle) const { return GetAssetMetadata(assetHandle).FilePath; }
    private:
        AssetMap m_LoadedAssets;
        AssetRegistry m_AssetRegistry;
    };
}
