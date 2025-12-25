#include "HRpch.h"
#include "TextureImporter.h"

#include "stb_image.h"
#include "HRealEngine/Renderer/Texture.h"


namespace HRealEngine
{
    Ref<Texture2D> TextureImporter::LoadTexture(const std::filesystem::path& path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        Buffer data;
        data.Data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
        
        if (data.Data == nullptr)
        {
            LOG_CORE_ERROR("Failed to load texture image from path: {}", path.string());
            return nullptr;
        }
        data.Size = width * height * channels;

        TextureSpecification spec;
        spec.Width = width;
        spec.Height = height;
        switch (channels)
        {
            case 3:
                spec.Format = ImageFormat::RGB8;
                break;
            case 4:
                spec.Format = ImageFormat::RGBA8;
                break;
        }
        Ref<Texture2D> textureAsset = Texture2D::Create(spec, data);
        data.Release();
        return textureAsset;
    }
}
