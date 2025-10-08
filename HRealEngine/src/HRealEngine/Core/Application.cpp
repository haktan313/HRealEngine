

#include "HRpch.h"
#include "Application.h"

#include <filesystem>

#include "HRealEngine/Renderer/Renderer.h"
#include "HRealEngine/Scripting/ScriptEngine.h"
#include "HRealEngine/Utils/PlatformUtils.h"

namespace HRealEngine
{
	Application* Application::s_InstanceOfApp = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_ApplicationSpecification(specification)
	{
		HREALENGINE_CORE_DEBUGBREAK(!s_InstanceOfApp, "App already exist!");
		s_InstanceOfApp = this;
		
		if (!m_ApplicationSpecification.WorkingDirectory.empty())
			std::filesystem::current_path(m_ApplicationSpecification.WorkingDirectory);
		
		m_Window = Window::Create(WindowSettings(m_ApplicationSpecification.Name));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		ScriptEngine::Init();
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}
	Application::~Application()
	{
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}
	void Application::Run()
	{
		while (m_bRunning)
		{
			float time = Time::GetTime();
			Timestep timeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_bMinimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timeStep);
				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)	
					layer->OnImGuiRender();
				m_ImGuiLayer->End();
			}
			m_Window->OnUpdate();
		}
	}
	void Application::OnEvent(EventBase& eventRef)
	{
		EventDispatcher dispatcher(eventRef);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
		
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			if (eventRef.m_bHandled) 
				break;
			(*--it)->OnEvent(eventRef);
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.emplace_back(function);
	}

	bool Application::OnWindowClose(WindowCloseEvent& eventRef)
	{
		m_bRunning = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& eventRef)
	{
		if (eventRef.GetWidth() == 0 || eventRef.GetHeight() == 0)
		{
			m_bMinimized = true;
			return false;
		}
		m_bMinimized = false;
		Renderer::OnWindowResize(eventRef.GetWidth(), eventRef.GetHeight());
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock lock(m_MainThreadQueueMutex);
		for (auto& func : m_MainThreadQueue)
			func();
		m_MainThreadQueue.clear();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}
} 
