#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "EnemyAI.h"
#include "Editor/NodeEditorApp.h"

class App
{
public:
    App();
    ~App();
    
    void Run();
    bool Init();
    void Shutdown();

    static void SizeCallback(GLFWwindow* window, int width, int height);

    static App* Get() { return s_Instance; }
    GLFWwindow* GetWindow() const { return m_Window; }
private:
    EnemyAI* m_EnemyAI;
    GLFWwindow* m_Window;
    ImGuiContext* m_ImGuiContext;
    
    static App* s_Instance;
};
