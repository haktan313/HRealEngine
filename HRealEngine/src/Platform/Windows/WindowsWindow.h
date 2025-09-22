
//WindowsWindow.h
#pragma once
#include <GLFW/glfw3.h>
#include "HRealEngine/Core/Window.h"
#include "HRealEngine/Renderer/GraphicsContext.h"

namespace HRealEngine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSettings& settings);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_WindowData.Width; }
		unsigned int GetHeight() const override { return m_WindowData.Height; }

		void SetEventCallback(const EventCallbackFn& callback) { m_WindowData.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual void* GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowSettings& settings);
		virtual void SetupGLFWCallbacks();
		virtual void Shutdown();

		GLFWwindow* m_Window;
		GraphicsContext* m_Context;
		struct WindowData
		{
			unsigned int Width;
			unsigned int Height;
			std::string Title;
			bool VSync;
			EventCallbackFn EventCallback;
		};
		WindowData m_WindowData;
	};
}

