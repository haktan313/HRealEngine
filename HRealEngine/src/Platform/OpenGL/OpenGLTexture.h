

#pragma once
#include "OpenGLShader.h"
#include "HRealEngine/Renderer/Texture.h"

namespace HRealEngine
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(uint32_t width, uint32_t height);
        OpenGLTexture2D(const std::string& path);
        virtual ~OpenGLTexture2D();

        void Bind(uint32_t slot = 0) const override;
        bool IsLoaded() const override { return m_bIsLoaded; }

        void SetData(void* data, uint32_t size) override;

        uint32_t GetWidth() const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint32_t GetRendererID() const override { return m_RendererID; }
        const std::string& GetPath() const override { return m_FilePath; }

        bool operator==(const Texture& other) const override {return m_RendererID == other.GetRendererID(); }
    private:
        bool m_bIsLoaded = false;
        std::string m_FilePath;
        uint32_t m_RendererID;
        uint32_t m_Width, m_Height;
        GLenum m_IternalFormat, m_DataFormat;
    };
}
