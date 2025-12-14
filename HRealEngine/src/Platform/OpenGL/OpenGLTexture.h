

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
    class OpenGLTexture3D : public Texture3D
    {
    public:
        OpenGLTexture3D(uint32_t width, uint32_t height, uint32_t depth);
        OpenGLTexture3D(const std::string& path); // Genelde 3D texture tek path ile olmaz ama interface geregi koyuyoruz
        virtual ~OpenGLTexture3D();

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        
        virtual const std::string& GetPath() const override { return m_FilePath; }
        
        virtual void SetData(void* data, uint32_t size) override;
        virtual void Bind(uint32_t slot = 0) const override;
        virtual bool IsLoaded() const override { return true; } // Simdilik true donelim

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == ((OpenGLTexture3D&)other).m_RendererID;
        }
    private:
        std::string m_FilePath;
        uint32_t m_Width, m_Height, m_Depth;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;
    };
}
