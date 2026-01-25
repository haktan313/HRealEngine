

#pragma once
#include "OpenGLShader.h"
#include "glad/glad.h"
#include "HRealEngine/Renderer/Texture.h"

namespace HRealEngine
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        /*OpenGLTexture2D(uint32_t width, uint32_t height);
        OpenGLTexture2D(const std::string& path);*/
        OpenGLTexture2D(const TextureSpecification& spec, Buffer initialData = Buffer());
        virtual ~OpenGLTexture2D();

        void Bind(uint32_t slot = 0) const override;
        bool IsLoaded() const override { return m_bIsLoaded; }

        //void SetData(void* data, uint32_t size) override;
        void SetData(Buffer data) override;
        void ApplySampling(bool enableMipmaps, int minFilter, int magFilter) override;

        uint32_t GetWidth() const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint32_t GetRendererID() const override { return m_RendererID; }
        //const std::string& GetPath() const override { return m_FilePath; }

        bool operator==(const Texture& other) const override {return m_RendererID == other.GetRendererID(); }
    private:
        TextureSpecification m_Specification;
        
        bool m_bIsLoaded = false;
        //std::string m_FilePath;
        uint32_t m_RendererID;
        uint32_t m_Width, m_Height;
        GLenum m_InternalFormat, m_DataFormat;
        uint32_t m_MipLevels = 1;
    };
}
