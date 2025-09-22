
#include "HRpch.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace HRealEngine
{
    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
