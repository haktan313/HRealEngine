
//OpenGLContext.h
#pragma once
#include "HRealEngine/Renderer/GraphicsContext.h"
#include "Platform/Windows/WindowsWindow.h"

namespace HRealEngine
{
    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* windowHandle);
        virtual void Init() override;
        virtual void SwapBuffers() override;
    private:
        GLFWwindow* windowRef;
    };
}
