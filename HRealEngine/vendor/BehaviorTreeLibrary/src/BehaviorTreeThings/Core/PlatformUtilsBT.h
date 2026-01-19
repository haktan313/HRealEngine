#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class PlatformUtilsBT
{
public:
    static std::string OpenFile(const char* filter);
    static std::string SaveFile(const char* filter);

    static void SetWindow(GLFWwindow* window) { s_Window = window; }
private:
    static GLFWwindow* s_Window;
};
