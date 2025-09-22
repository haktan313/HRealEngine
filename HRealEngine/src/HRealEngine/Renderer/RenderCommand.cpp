

#include "HRpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace HRealEngine
{
    Scope<RendererAPI> RenderCommand::m_RendererAPI = RendererAPI::Create();
}
