#include "HRpch.h"
#include "EditorAssetManager.h"
#include <fstream>
#include "AssetImporter.h"
#include <yaml-cpp/yaml.h>

#include "HRealEngine/Project/Project.h"

namespace HRealEngine
{
    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
        { ".hrs", AssetType::Scene },
        { ".png", AssetType::Texture },
        { ".jpg", AssetType::Texture },
        { ".jpeg", AssetType::Texture }
    };
    static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
    {
        if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
        {
            LOG_CORE_WARN("Could not find AssetType for {}", extension.string());
            return AssetType::None;
        }

        return s_AssetExtensionMap.at(extension);
    }
    YAML::Emitter& operator<<(YAML::Emitter& out, const std::string_view& v)
    {
        out << std::string(v.data(), v.size());
        return out;
    }
    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle assetHandle)
    {
        if (!IsAssetHandleValid(assetHandle))
            return nullptr;
        Ref<Asset> asset;
        if (IsAssetLoaded(assetHandle))
        {
            asset = m_LoadedAssets[assetHandle];
        }
        else
        {
            const AssetMetadata& metaData = GetAssetMetadata(assetHandle);
            asset = AssetImporter::ImportAsset(assetHandle, metaData);
            if (!asset)
                LOG_CORE_ERROR("Failed to load asset: {}", metaData.FilePath.string());
            m_LoadedAssets[assetHandle] = asset;
        }
        return asset;
    }

    AssetType EditorAssetManager::GetAssetType(AssetHandle assetHandle) const
    {
        if (!IsAssetHandleValid(assetHandle))
            return AssetType::None;

        return m_AssetRegistry.at(assetHandle).Type;
    }

    void EditorAssetManager::ImportAsset(const std::filesystem::path& filePath)
    {
        AssetHandle newAssetHandle;
        AssetMetadata newAssetMetaData;
        newAssetMetaData.FilePath = filePath;
        newAssetMetaData.Type = GetAssetTypeFromFileExtension(filePath.extension());
        HREALENGINE_CORE_DEBUGBREAK(metadata.Type != AssetType::None);

        Ref<Asset> newAsset = AssetImporter::ImportAsset(newAssetHandle, newAssetMetaData);
        if (newAsset)
        {
            newAsset->Handle = newAssetHandle;
            m_LoadedAssets[newAssetHandle] = newAsset;
            m_AssetRegistry[newAssetHandle] = newAssetMetaData;
            SerializeAssetRegistry();
        }
    }

    void EditorAssetManager::SerializeAssetRegistry()
    {
        auto path = Project::GetAssetRegistryPath();
        YAML::Emitter out;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "AssetRegistry" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& [handle, metadata] : m_AssetRegistry)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;
                std::string filePath = metadata.FilePath.generic_string();
                out << YAML::Key << "FilePath" << YAML::Value << filePath;
                out << YAML::Key << "Type" << YAML::Value << static_cast<int>(metadata.Type);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        std::ofstream fout(path);
        fout << out.c_str();
    }

    bool EditorAssetManager::DeserializeAssetRegistry()
    {
        auto path = Project::GetAssetRegistryPath();
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            HREALENGINE_CORE_DEBUGBREAK("Failed to load asset registry file '{0}'\n     {1}", path, e.what());
            return false;
        }
        auto rootNde = data["AssetRegistry"];
        if (!rootNde)
            return false;
        for (const auto& assetNode : rootNde)
        {
            AssetHandle handle = assetNode["Handle"].as<uint64_t>();
            auto& metadata = m_AssetRegistry[handle];
            std::string filePath = assetNode["FilePath"].as<std::string>();
            metadata.FilePath = filePath;
            metadata.Type = AssetTypeFromString(assetNode["Type"].as<std::string>());
        }
        return true;
    }

    const AssetMetadata& EditorAssetManager::GetAssetMetadata(AssetHandle assetHandle) const
    {
        static AssetMetadata dummyMetadata;
        auto it = m_AssetRegistry.find(assetHandle);
        if (it != m_AssetRegistry.end())
            return it->second;
        return dummyMetadata;
    }
}
