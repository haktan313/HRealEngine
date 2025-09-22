
#include "HRpch.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace HRealEngine
{
    RendererAPI::API RendererAPI::m_CurrentAPI = RendererAPI::API::OpenGL;

    Scope<RendererAPI> RendererAPI::Create()
    {
        switch (m_CurrentAPI)
        {
            case API::None:    HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is currently not supported!"); return nullptr;
            case API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
