
//RenderCommand.cpp
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace HRealEngine
{
    RendererAPI* RenderCommand::rendererAPI = new OpenGLRendererAPI;
}
