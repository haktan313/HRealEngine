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
        { ".jpeg", AssetType::Texture },
        {".obj", AssetType::Mesh },
        {".hmesh", AssetType::Mesh },
        {".hmat", AssetType::Material},
        {".mtl", AssetType::Material},
        {".btree", AssetType::BehaviorTree},
        {".hskeleton", AssetType::Skeleton}
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
    /*YAML::Emitter& operator<<(YAML::Emitter& out, const std::string_view& v)
    {
        out << std::string(v.data(), v.size());
        return out;
    }*/
    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle assetHandle)
    {
        if (!IsAssetHandleValid(assetHandle))
        {
            LOG_CORE_ERROR("Invalid asset handle: {}", (int)assetHandle);
            return nullptr;
        }
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

    AssetHandle EditorAssetManager::GetHandleFromPath(const std::filesystem::path& relPath) const
    {
        std::filesystem::path norm = relPath.lexically_normal();

        for (const auto& [handle, meta] : m_AssetRegistry)
        {
            if (meta.FilePath.lexically_normal() == norm)
                return handle;
        }
        return 0;
    }

    Ref<Asset> EditorAssetManager::ReloadAsset(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle))
            return nullptr;
        
        m_LoadedAssets.erase(handle);
        Ref<Asset> reloaded = GetAsset(handle);
        return reloaded;
    }

    void EditorAssetManager::ImportAsset(const std::filesystem::path& filePath)
    {
        const std::filesystem::path assetRoot = Project::GetAssetDirectory();
        std::filesystem::path absPath = std::filesystem::weakly_canonical(filePath);

        std::filesystem::path relPath;
        {
            std::error_code ec;
            std::filesystem::path rel = std::filesystem::relative(absPath, assetRoot, ec);
            
            bool outsideAssets = ec || rel.empty() || (*rel.begin() == "..");
            if (!outsideAssets)
                relPath = rel;
        }

        if (relPath.empty())
        {
            const std::filesystem::path importDir = assetRoot / "Imported";
            std::filesystem::create_directories(importDir);
            
            std::filesystem::path dst = importDir / absPath.filename();
            if (std::filesystem::exists(dst))
            {
                int suffix = 1;
                do
                {
                    dst = importDir / (absPath.stem().string() + "_" + std::to_string(suffix) + absPath.extension().string());
                    suffix++;
                }
                while (std::filesystem::exists(dst));
            }

            std::error_code ec;
            std::filesystem::copy_file(absPath, dst, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                LOG_CORE_ERROR("Failed to copy imported asset: {} -> {} ({})", absPath.string(), dst.string(), ec.message());
                return;
            }

            absPath = dst;
            relPath = std::filesystem::relative(dst, assetRoot);
        }

        for (const auto& [handle, metadata] : m_AssetRegistry)
            if (metadata.FilePath == relPath)
                return;

        AssetHandle newHandle;
        AssetMetadata meta;
        meta.FilePath = relPath;
        meta.Type = GetAssetTypeFromFileExtension(relPath.extension());
        HREALENGINE_CORE_DEBUGBREAK(meta.Type != AssetType::None);

        Ref<Asset> asset = AssetImporter::ImportAsset(newHandle, meta);
        if (!asset)
            return;

        asset->Handle = newHandle;
        m_LoadedAssets[newHandle] = asset;
        m_AssetRegistry[newHandle] = meta;
        SerializeAssetRegistry();

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
                out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
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
        catch (const YAML::Exception& e)
        {
            LOG_CORE_ERROR("Failed to load asset registry '{}': {}", path.string(), e.what());
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
