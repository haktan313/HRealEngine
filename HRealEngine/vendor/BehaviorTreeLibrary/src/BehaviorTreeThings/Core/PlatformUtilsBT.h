#pragma once
#include <string>

struct GLFWwindow;

class PlatformUtilsBT
{
public:
    static std::string OpenFile(const char* filter);
    static std::string SaveFile(const char* filter);

    static void SetWindow(GLFWwindow* window) { s_Window = window; }
private:
    static GLFWwindow* s_Window;
};
