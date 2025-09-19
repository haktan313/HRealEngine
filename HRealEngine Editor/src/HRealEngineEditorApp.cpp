
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
		HRealEngineEditorApp() : Application("HRealEngine Editor")
		{
			PushLayer(new EditorLayer());
		}
		~HRealEngineEditorApp()
		{

		}
	};

	Application* CreateApplication()
	{
		return new HRealEngineEditorApp();
	}
}