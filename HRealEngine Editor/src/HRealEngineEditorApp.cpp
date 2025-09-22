
//HRealEngineEditorApp.cpp
#include "HRealEngine.h"
#include "HRealEngine/Core/EntryPoint.h"
#include "EditorLayer.h"
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace HRealEngine
{
	class HRealEngineEditorApp : public Application
	{
	public:
		HRealEngineEditorApp(const ApplicationSpecification& spec) : Application(spec)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(AppCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "HRealEngine Editor";
		spec.CommandLineArgs = args;
		
		return new HRealEngineEditorApp(spec); 
	}
}