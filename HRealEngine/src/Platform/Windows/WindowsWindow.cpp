
//WindowsWindow.cpp
#include "HRpch.h"
#include "WindowsWindow.h"
#include "HRealEngine/Core/Core.h"
#include <HRealEngine/Events/EventBase.h>
#include <HRealEngine/Events/AppEvent.h>
#include <HRealEngine/Events/KeyEvent.h>
#include <HRealEngine/Events/MouseEvent.h>
#include <glad/glad.h>
#include "Platform/OpenGL/OpenGLContext.h"

namespace HRealEngine
{
	static bool GLFWInitialized = false;

	static void GLFWSetErrorCallback(int error, const char* description)
	{
		LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window* Window::Create(const WindowSettings& settings)
	{
		return new WindowsWindow(settings);
	}

	WindowsWindow::WindowsWindow(const WindowSettings& settings)
	{
		Init(settings);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		contextRef->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if(enabled)
		{
			glfwSwapInterval(1);
			LOG_CORE_INFO("VSync enabled");
		}
		else
		{
			glfwSwapInterval(0);
			LOG_CORE_INFO("VSync disabled");
		}
		windowDataRef.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return windowDataRef.VSync;
	}

	void WindowsWindow::Init(const WindowSettings& settings)
	{
		windowDataRef.Height = settings.Height;
		windowDataRef.Width = settings.Width;
		windowDataRef.Title = settings.Title;
		LOG_CORE_INFO("Creating window: {0} ({1}, {2})", windowDataRef.Title, windowDataRef.Width, windowDataRef.Height);
		
		if(!GLFWInitialized)
		{
			int result = glfwInit();
			HREALENGINE_CORE_DEBUGBREAK(result, "Failed to initialize GLFW");
			glfwSetErrorCallback(GLFWSetErrorCallback);
			GLFWInitialized = true;
		}

		windowRef = glfwCreateWindow(windowDataRef.Width, windowDataRef.Height, windowDataRef.Title.c_str(), nullptr, nullptr);
		
		contextRef = new OpenGLContext(windowRef);
		contextRef->Init();

		glfwSetWindowUserPointer(windowRef, &windowDataRef);
		SetVSync(true);
		SetupGLFWCallbacks();
	}

	void WindowsWindow::SetupGLFWCallbacks()
	{
		glfwSetWindowSizeCallback(windowRef, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});
		glfwSetWindowCloseCallback(windowRef, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});
		glfwSetKeyCallback(windowRef, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});
		glfwSetCharCallback(windowRef, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});
		glfwSetMouseButtonCallback(windowRef, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});
		glfwSetCursorPosCallback(windowRef, [](GLFWwindow* window, double xpos, double ypos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xpos, (float)ypos);
			data.EventCallback(event);
		});
		glfwSetScrollCallback(windowRef, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float)xoffset, (float)yoffset);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(windowRef);
	}
};