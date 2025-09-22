
//OpenGLTexture.h
#pragma once
#include "OpenGLShader.h"
#include "HRealEngine/Renderer/Texture.h"
#include "glad/glad.h"

namespace HRealEngine
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(uint32_t width, uint32_t height);
        OpenGLTexture2D(const std::string& path);
        virtual ~OpenGLTexture2D();

        virtual void Bind(uint32_t slot = 0) const override;
        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual void SetData(void* data, uint32_t size) override;

        virtual uint32_t GetWidth() const override { return widthRef; }
        virtual uint32_t GetHeight() const override { return heightRef; }
        virtual uint32_t GetRendererID() const override { return rendererID; }
        virtual const std::string& GetPath() const override { return filePath; }

        virtual bool operator==(const Texture& other) const override {return rendererID == other.GetRendererID(); }
    private:
        bool m_IsLoaded = false;
        std::string filePath;
        uint32_t rendererID;
        uint32_t widthRef, heightRef;
        GLenum m_internalFormat, m_dataFormat;
    };
}
