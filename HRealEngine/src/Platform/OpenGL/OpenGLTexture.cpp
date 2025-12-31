

#include "HRpch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>
#include <glad/glad.h>

namespace HRealEngine
{
    /*OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
    {
        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, width, height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_FilePath(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); 
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        
        if (data)
        {
            m_bIsLoaded = true;
            m_Width = width;
            m_Height = height;
            GLenum internalFormat = 0, dataFormat = 0;
            if (channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }else if (channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }
            m_InternalFormat = internalFormat;
            m_DataFormat = dataFormat;

            HREALENGINE_CORE_DEBUGBREAK(internalFormat & dataFormat, "Texture format not supported!");
            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, internalFormat, width, height);

            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }*/

    static GLenum ImageFormatToGLDataFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB8:  return GL_RGB;
        case ImageFormat::RGBA8: return GL_RGBA;
        }

        HREALENGINE_CORE_DEBUGBREAK(false);
        return 0;
    }

    static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB8:  return GL_RGB8;
        case ImageFormat::RGBA8: return GL_RGBA8;
        }

        HREALENGINE_CORE_DEBUGBREAK(false);
        return 0;
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec, Buffer initialData) : m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        m_InternalFormat = ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (initialData)
            SetData(initialData);
    }

    void OpenGLTexture2D::SetData(Buffer data)
    {
        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        HREALENGINE_CORE_DEBUGBREAK(data.Size == m_Width * m_Height * bpp, "Data must be entire texture!");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
        m_bIsLoaded = true;
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

    /*void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        HREALENGINE_CORE_DEBUGBREAK(size == widthRef * heightRef * bpp, "Data must be entire texture!");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    }*/
    
}
