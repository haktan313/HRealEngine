
//SandboxApp.cpp
#include "HRealEngine.h"
#include "HRealEngine/Core/EntryPoint.h"
#include "Sandbox2D.h"
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

class Sandbox : public HRealEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new Sandbox2D());
	}
	~Sandbox()
	{

	}
};

HRealEngine::Application* HRealEngine::CreateApplication()
{
	return new Sandbox();
}