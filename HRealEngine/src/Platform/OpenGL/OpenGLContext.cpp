

#include "HRpch.h"
#include "OpenGLContext.h"
#include "glad/glad.h"

namespace HRealEngine
{
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : windowRef(windowHandle)
    {
        HREALENGINE_CORE_DEBUGBREAK(windowHandle, "Window handle is null");
    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(windowRef);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        HREALENGINE_CORE_DEBUGBREAK(status, "Failed to initialize GLAD");
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(windowRef);
    }
}
