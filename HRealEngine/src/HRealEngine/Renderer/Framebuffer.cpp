
//Framebuffer.cpp
#include "Framebuffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace HRealEngine
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
