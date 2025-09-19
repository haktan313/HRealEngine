
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

		inline unsigned int GetWidth() const override { return windowDataRef.Width; }
		inline unsigned int GetHeight() const override { return windowDataRef.Height; }

		inline void SetEventCallback(const EventCallbackFn& callback) { windowDataRef.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const { return windowRef; }
	private:
		virtual void Init(const WindowSettings& settings);
		virtual void SetupGLFWCallbacks();
		virtual void Shutdown();

		GLFWwindow* windowRef;
		GraphicsContext* contextRef;
		struct WindowData
		{
			unsigned int Width;
			unsigned int Height;
			std::string Title;
			bool VSync;
			EventCallbackFn EventCallback;
		};
		WindowData windowDataRef;
	};
}

