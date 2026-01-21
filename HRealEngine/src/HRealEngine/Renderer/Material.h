#pragma once
#include <filesystem>
#include <unordered_map>

#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Texture.h"
#include "HRealEngine/Renderer/Shader.h"

namespace HRealEngine
{
    class HMaterial : public Asset
    {
    public:
        virtual ~HMaterial() = default;
        
        glm::vec4 Color = {1,1,1,1};

        AssetHandle AlbedoTextureHandle = 0;
        AssetHandle SpecularTextureHandle = 0;
        AssetHandle NormalTextureHandle = 0;
        float Shininess = 32.0f;
        
        mutable Ref<Texture2D> AlbedoTextureCache   = nullptr;
        mutable Ref<Texture2D> SpecularTextureCache = nullptr;
        mutable Ref<Texture2D> NormalTextureCache   = nullptr;

        void Apply(const Ref<Shader>& shader) const
        {
            shader->SetFloat4("u_Color", Color);
            shader->SetFloat("u_Shininess", Shininess);
            
            bool hasAlbedo = false;
            if (AlbedoTextureHandle != 0 && AssetManager::IsAssetHandleValid(AlbedoTextureHandle))
            {
                if (!AlbedoTextureCache || !AlbedoTextureCache->IsLoaded())
                    AlbedoTextureCache = AssetManager::GetAsset<Texture2D>(AlbedoTextureHandle);

                if (AlbedoTextureCache && AlbedoTextureCache->IsLoaded())
                {
                    AlbedoTextureCache->Bind(0);
                    shader->SetInt("u_Albedo", 0);
                    hasAlbedo = true;
                }
            }
            shader->SetInt("u_HasAlbedo", hasAlbedo ? 1 : 0);
            
            bool hasSpec = false;
            if (SpecularTextureHandle != 0 && AssetManager::IsAssetHandleValid(SpecularTextureHandle))
            {
                if (!SpecularTextureCache || !SpecularTextureCache->IsLoaded())
                    SpecularTextureCache = AssetManager::GetAsset<Texture2D>(SpecularTextureHandle);

                if (SpecularTextureCache && SpecularTextureCache->IsLoaded())
                {
                    SpecularTextureCache->Bind(1);
                    shader->SetInt("u_Specular", 1);
                    hasSpec = true;
                }
            }
            shader->SetInt("u_HasSpecular", hasSpec ? 1 : 0);

            bool hasNormal = false;
            if (NormalTextureHandle != 0 && AssetManager::IsAssetHandleValid(NormalTextureHandle))
            {
                if (!NormalTextureCache || !NormalTextureCache->IsLoaded())
                    NormalTextureCache = AssetManager::GetAsset<Texture2D>(NormalTextureHandle);

                if (NormalTextureCache && NormalTextureCache->IsLoaded())
                {
                    NormalTextureCache->Bind(2);
                    shader->SetInt("u_Normal", 2);
                    hasNormal = true;
                }
            }
            shader->SetInt("u_HasNormal", hasNormal ? 1 : 0);

        }

        static AssetType GetStaticType() { return AssetType::Material; }
        AssetType GetType() const override { return GetStaticType(); }
    };

    class MaterialLibrary
    {
    public:
        static Ref<HMaterial> GetOrLoad(const std::filesystem::path& relHmatPath, const std::filesystem::path& assetsRoot);
        static void Clear();

    private:
        static Ref<HMaterial> LoadFromFile(const std::filesystem::path& absPath, const std::filesystem::path& assetsRoot);

        static std::unordered_map<std::filesystem::path, Ref<HMaterial>> s_Cache;
    };
}
