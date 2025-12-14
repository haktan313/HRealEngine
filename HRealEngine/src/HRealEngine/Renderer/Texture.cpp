
#include "HRpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace HRealEngine
{
    Ref<Texture2D> Texture2D::Create(const std::string& filePath)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(filePath);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(width, height);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture3D> Texture3D::Create(const std::string& filePath)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture3D>(filePath);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture3D> Texture3D::Create(uint32_t width, uint32_t height, uint32_t depth)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture3D>(width, height, depth);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
