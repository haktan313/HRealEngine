
//Application.h
#pragma once
#include "Core.h"
#include "Window.h"
#include "Layer.h"
#include "LayerStack.h"
#include "HRealEngine/Events/EventBase.h"
#include "HRealEngine/Events/AppEvent.h"
#include "HRealEngine/ImGui/ImGuiLayer.h"
#include "HRealEngine/Renderer/VertexArray.h"

namespace HRealEngine
{
	struct AppCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			HREALENGINE_CORE_DEBUGBREAK(index < Count);
			return Args[index];
		}
	};
	class HREALENGINE_API Application
	{
	public:
		Application(const std::string& name = "HRealEngine App", AppCommandLineArgs args = AppCommandLineArgs());
		virtual ~Application();

		void Run();
		void OnEvent(EventBase& eventRef);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void Close() { bRunning = false; }

		inline Window& GetWindow() { return *windowRef; }
		ImGuiLayer* GetImGuiLayer() { return imGuiLayerRef; }
		inline static Application& Get() { return *InstanceOfApp; }

		AppCommandLineArgs GetCommandLineArgs() const { return commandLineArgs; }
	private:
		bool OnWindowClose(WindowCloseEvent& eventRef);
		bool OnWindowResize(WindowResizeEvent& eventRef);

		AppCommandLineArgs commandLineArgs;
		std::unique_ptr<Window> windowRef;
		ImGuiLayer* imGuiLayerRef;
		bool bRunning = true;
		bool bMinimized = false;
		LayerStack layerStack;
		static Application* InstanceOfApp;
		float lastFrameTime = 0.0f;
		
	};
	Application* CreateApplication(AppCommandLineArgs args); 
}
