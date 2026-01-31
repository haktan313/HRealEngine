
#include "HRpch.h"
#include "OpenGLFramebuffer.h"
#include <glad/glad.h>

namespace HRealEngine
{
    static uint32_t s_maxFramebufferSize = 8192;

    static bool IsDepthFormat(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
        }
        return false;
    }

    static GLenum TextureTarget(bool bMultisample)
    {
        return bMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    }
    
    static void CreateTextures(bool bMultisample, uint32_t* outID, uint32_t count)
    {
        glCreateTextures(TextureTarget(bMultisample), count, outID);
    }

    static void BindTexture(bool bMultisample, uint32_t id)
    {
        glBindTexture(TextureTarget(bMultisample), id);
    }

    static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
    {
        bool bMultisample = samples > 1;
        if (bMultisample)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(bMultisample), id, 0);
    }

    static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
    {
        bool bMultisample = samples > 1;
        if (bMultisample)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(bMultisample), id, 0);
    }

    static GLenum HRETextureFormatToGL(FramebufferTextureFormat format)
    {
        switch (format)
        {
        case FramebufferTextureFormat::RGBA8:         return GL_RGBA8;
        case FramebufferTextureFormat::RED_INTEGER:    return GL_RED_INTEGER;
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown Texture Format!");
        return 0;
    }

    
    
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
    {
        for (auto spec : m_Specification.Attachments.Attachments)    
        {
            if (!IsDepthFormat(spec.TextureFormat))
                m_ColorAttachmentSpecs.emplace_back(spec);
            else
                m_DepthAttachmentSpec = spec;
        }
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_rendererID);
        glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_rendererID)
        {
            glDeleteFramebuffers(1, &m_rendererID);
            glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }
        
        glCreateFramebuffers(1, &m_rendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

        bool bMultisample = m_Specification.Samples > 1;
        if (m_ColorAttachmentSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            CreateTextures(bMultisample, m_ColorAttachments.data(), static_cast<uint32_t>(m_ColorAttachments.size()));
            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                BindTexture(bMultisample, m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].TextureFormat)
                {
                    case FramebufferTextureFormat::RGBA8:
                        AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height,
                                           static_cast<int>(i));
                        break;
                    case FramebufferTextureFormat::RED_INTEGER:
                        AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height,
                                           static_cast<int>(i));
                        break;
                }
            }
        }
        if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            CreateTextures(bMultisample, &m_DepthAttachment, 1);
            BindTexture(bMultisample, m_DepthAttachment);
            switch (m_DepthAttachmentSpec.TextureFormat)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
            }
        }
        if (m_ColorAttachments.size() > 1)
        {
            HREALENGINE_CORE_DEBUGBREAK(m_ColorAttachments.size() > 4, "We only support 4 color attachments for now!");
            GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }
        
        HREALENGINE_CORE_DEBUGBREAK(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);

        //int value = -1;
        glClearTexImage(m_ColorAttachments[1], 0, GL_RED_INTEGER, GL_INT, (void*)0);
    }

    void OpenGLFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_maxFramebufferSize || height > s_maxFramebufferSize)
        {
            LOG_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
            return;
        }
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

    int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixelData;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        return pixelData;
    }

    void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
    {
        auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
        glClearTexImage(m_ColorAttachments[attachmentIndex], 0, HRETextureFormatToGL(spec.TextureFormat), GL_INT, &value);
    }
}
