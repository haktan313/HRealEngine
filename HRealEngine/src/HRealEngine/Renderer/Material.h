#pragma once
#include <filesystem>
#include <unordered_map>

#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Texture.h"
#include "HRealEngine/Renderer/Shader.h"

namespace HRealEngine
{
    struct HMaterial
    {
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> AlbedoTexture = nullptr;

        void Apply(const Ref<Shader>& shader) const
        {
            shader->SetFloat4("u_Color", Color);

            if (AlbedoTexture && AlbedoTexture->IsLoaded())
            {
                AlbedoTexture->Bind(0);
                shader->SetInt("u_Albedo", 0);
                shader->SetInt("u_HasAlbedo", 1);
            }
            else
            {
                shader->SetInt("u_HasAlbedo", 0);
            }
        }
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
