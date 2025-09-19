
//Application.cpp
#include "HRpch.h"
#include "Application.h"
#include "Core.h"
#include "GLFW/glfw3.h"
#include "HRealEngine/Renderer/Renderer.h"
#include "HRealEngine/Core/Timestep.h"

namespace HRealEngine
{
	Application* Application::InstanceOfApp = nullptr;

	Application::Application(const std::string& name)
	{
		HREALENGINE_CORE_DEBUGBREAK(!InstanceOfApp, "App already exist!");
		InstanceOfApp = this;
		
		windowRef = std::unique_ptr<Window>(Window::Create(WindowSettings(name)));
		windowRef->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		
		imGuiLayerRef = new ImGuiLayer();
		PushOverlay(imGuiLayerRef);
	}
	Application::~Application()
	{
	}
	void Application::Run()
	{
		while (bRunning)
		{
			float time = (float)glfwGetTime();//Platform::GetTime();
			Timestep timestep = time - lastFrameTime;
			lastFrameTime = time;

			if (!bMinimized)
			{
				for (Layer* layer : layerStack)
				{
					layer->OnUpdate(timestep);
				}
			}
			
			imGuiLayerRef->Begin();
			for (Layer* layer : layerStack)	
			{
				layer->OnImGuiRender();
			}
			imGuiLayerRef->End();

			windowRef->OnUpdate();
		}
	}
	void Application::OnEvent(EventBase& eventRef)
	{
		EventDispatcher dispatcher(eventRef);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		LOG_CORE_TRACE("{0}", eventRef.ToString());
		for (auto it = layerStack.end(); it != layerStack.begin();)
		{
			if (eventRef.bHandled) 
			{
				break;
			}
			(*--it)->OnEvent(eventRef);
		}
	}
	bool Application::OnWindowClose(WindowCloseEvent& eventRef)
	{
		bRunning = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& eventRef)
	{
		if (eventRef.GetWidth() == 0 || eventRef.GetHeight() == 0)
		{
			bMinimized = true;
			return false;
		}
		bMinimized = false;
		Renderer::OnwindowResize(eventRef.GetWidth(), eventRef.GetHeight());
		return false;
	}
	void Application::PushLayer(Layer* layer)
	{
		layerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* overlay)
	{
		layerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}
} 
