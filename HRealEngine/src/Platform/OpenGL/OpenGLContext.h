

#pragma once
#include "Platform/Windows/WindowsWindow.h"

namespace HRealEngine
{
    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* windowHandle);
        void Init() override;
        void SwapBuffers() override;
    private:
        GLFWwindow* windowRef;
    };
}
