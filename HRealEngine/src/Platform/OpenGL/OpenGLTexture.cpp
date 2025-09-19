
//OpenGLTexture.cpp
#include "OpenGLTexture.h"

#include "stb_image.h"
#include "HRealEngine/Core/Logger.h"

namespace HRealEngine
{
    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) : widthRef(width), heightRef(height)
    {
        m_internalFormat = GL_RGBA8;
        m_dataFormat = GL_RGBA;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
        glTextureStorage2D(rendererID, 1, m_internalFormat, width, height);

        glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : filePath(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); 
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        /*HREALENGINE_CORE_DEBUGBREAK(data, "Failed to load texture!");
        widthRef = width;
        heightRef = height;
 
        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else
            HREALENGINE_CORE_DEBUGBREAK(false, "Texture format not supported!");

        m_internalFormat = internalFormat;
        m_dataFormat = dataFormat;*/

        if (data)
        {
            m_IsLoaded = true;
            widthRef = width;
            heightRef = height;
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
            m_internalFormat = internalFormat;
            m_dataFormat = dataFormat;

            HREALENGINE_CORE_DEBUGBREAK(internalFormat & dataFormat, "Texture format not supported!");
            glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
            glTextureStorage2D(rendererID, 1, internalFormat, width, height);

            glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTextureSubImage2D(rendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        /*glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
        glTextureStorage2D(rendererID, 1, internalFormat, width, height);*/

        /*glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
        /*glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(rendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);*/
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &rendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, rendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
        HREALENGINE_CORE_DEBUGBREAK(size == widthRef * heightRef * bpp, "Data must be entire texture!");
        glTextureSubImage2D(rendererID, 0, 0, 0, widthRef, heightRef, m_dataFormat, GL_UNSIGNED_BYTE, data);
    }
}
