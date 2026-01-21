

#pragma once
#include "HRealEngine/Core/Core.h"
#include <string>
#include "glm/glm.hpp"
#include "HRealEngine/Asset/Asset.h"
#include "HRealEngine/Core/Buffer.h"

namespace HRealEngine
{
    enum class ImageFormat
    {
        None = 0,
        R8,
        RGB8,
        RGBA8,
        RGBA32F
    };

    enum class TextureFilter
    {
        Nearest,
        Linear
    };

    struct TextureSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        
        ImageFormat Format = ImageFormat::RGBA8;
        
        bool GenerateMips = true;
        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;
    };
    
    class Texture : public Asset
    {
    public:
        virtual ~Texture() = default;
        
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        //virtual void SetData(void* data, uint32_t size) = 0;
        virtual void SetData(Buffer data) = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;
        virtual bool IsLoaded() const = 0;
        //virtual const std::string& GetPath() const = 0;
        virtual bool operator==(const Texture& other) const = 0;
    };

    class Texture2D : public Texture
    {
    public:
        /*static Ref<Texture2D> Create(const std::string& filePath);
        static Ref<Texture2D> Create(uint32_t width, uint32_t height);*/
        static Ref<Texture2D> Create(const TextureSpecification& spec, Buffer initialData = Buffer());

        static AssetType GetStaticAssetType() { return AssetType::Texture; }
        virtual AssetType GetType() const override { return GetStaticAssetType(); }
    };
}
