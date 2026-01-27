
#pragma once
#include "Core.h"
#include "Window.h"
#include "Layer.h"
#include "LayerStack.h"
#include "HRealEngine/ImGui/ImGuiLayer.h"
#include "HRealEngine/Renderer/VertexArray.h"

#include <vector>
#include <functional>
#include <mutex>


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
	struct ApplicationSpecification
	{
		std::string Name = "HRealEngine Application";
		AppCommandLineArgs CommandLineArgs;

		std::filesystem::path EditorAssetsPath = "assets";
	};
	class HREALENGINE_API Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void OnEvent(EventBase& eventRef);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void Close() { m_bRunning = false; }

		Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		static Application& Get() { return *s_InstanceOfApp; }

		const ApplicationSpecification& GetSpecification() const { return m_ApplicationSpecification; }

		void SubmitToMainThread(const std::function<void()>& function);
		
	private:
		bool OnWindowClose(WindowCloseEvent& eventRef);
		bool OnWindowResize(WindowResizeEvent& eventRef);

		void ExecuteMainThreadQueue();

		ApplicationSpecification m_ApplicationSpecification;
		LayerStack m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;
		static Application* s_InstanceOfApp;

		Scope<Window> m_Window;
		
		bool m_bRunning = true;
		bool m_bMinimized = false;

		float m_LastFrameTime = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	};
	Application* CreateApplication(AppCommandLineArgs args); 
}
