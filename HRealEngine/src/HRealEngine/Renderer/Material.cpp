#include "HRpch.h"
#include "Material.h"

#include "HRealEngine/Core/Logger.h"
#include <yaml-cpp/yaml.h>

#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Asset/TextureImporter.h"

namespace HRealEngine
{
    std::unordered_map<std::filesystem::path, Ref<HMaterial>> MaterialLibrary::s_Cache;

    static std::filesystem::path MakeAbs(const std::filesystem::path& assetsRoot, const std::filesystem::path& maybeRel)
    {
        if (maybeRel.is_absolute())
            return maybeRel;

        return assetsRoot / maybeRel;
    }

    Ref<HMaterial> MaterialLibrary::GetOrLoad(const std::filesystem::path& relHmatPath, const std::filesystem::path& assetsRoot)
    {
        if (relHmatPath.empty())
            return nullptr;

        auto it = s_Cache.find(relHmatPath);
        if (it != s_Cache.end())
            return it->second;

        const std::filesystem::path absPath = MakeAbs(assetsRoot, relHmatPath);
        if (!std::filesystem::exists(absPath))
        {
            LOG_CORE_WARN("HMAT missing: {} (abs={})", relHmatPath.string(), absPath.string());
            return nullptr;
        }

        Ref<HMaterial> mat = LoadFromFile(absPath, assetsRoot);
        s_Cache[relHmatPath] = mat;
        return mat;
    }

    Ref<HMaterial> MaterialLibrary::LoadFromFile(const std::filesystem::path& absPath, const std::filesystem::path& assetsRoot)
    {
        LOG_CORE_WARN("Loading HMAT absPath={}", absPath.string());

        Ref<HMaterial> mat = CreateRef<HMaterial>();

        YAML::Node root;
        try
        {
            root = YAML::LoadFile(absPath.string());
        }
        catch (const std::exception& e)
        {
            LOG_CORE_ERROR("Failed to load HMAT YAML: {} ({})", absPath.string(), e.what());
            return mat;
        }
        
        if (root["BaseColor"] && root["BaseColor"].IsSequence() && root["BaseColor"].size() == 3)
        {
            mat->Color.r = root["BaseColor"][0].as<float>();
            mat->Color.g = root["BaseColor"][1].as<float>();
            mat->Color.b = root["BaseColor"][2].as<float>();
            mat->Color.a = 1.0f;
        }
        else if (root["Color"] && root["Color"].IsSequence() && root["Color"].size() == 4)
        {
            mat->Color.r = root["Color"][0].as<float>();
            mat->Color.g = root["Color"][1].as<float>();
            mat->Color.b = root["Color"][2].as<float>();
            mat->Color.a = root["Color"][3].as<float>();
        }

        if (root["AlbedoTextureHandle"])
        {
            AssetHandle h = (AssetHandle)root["AlbedoTextureHandle"].as<uint64_t>(0);
            mat->AlbedoTextureHandle = h;
        }

        if (root["SpecularTextureHandle"])
        {
            AssetHandle h = (AssetHandle)root["SpecularTextureHandle"].as<uint64_t>(0);
            mat->SpecularTextureHandle = h;
        }

        if (root["Shininess"])
        {
            mat->Shininess = root["Shininess"].as<float>(32.0f);
        }

        if (root["NormalTextureHandle"])
        {
            AssetHandle h = (AssetHandle)root["NormalTextureHandle"].as<uint64_t>(0);
            mat->NormalTextureHandle = h;
        }

        
        return mat;
    }

    void MaterialLibrary::Clear()
    {
        s_Cache.clear();
    }
}
