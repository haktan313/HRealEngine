#pragma once
#include "Asset.h"
#include "HRealEngine/Core/UUID.h"
#include "HRealEngine/Renderer/Texture.h"

namespace HRealEngine
{
    class TextureImporter
    {
    public:
        static Ref<Asset> ImportTexture(AssetHandle assetHandle, const AssetMetadata& metaData) { return LoadTexture(metaData.FilePath); }
        static Ref<Texture2D> LoadTexture(const std::filesystem::path& path);
    };
}
