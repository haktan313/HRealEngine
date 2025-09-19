
//Window.h
#pragma once
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Events/EventBase.h"
#include "HRpch.h"

namespace HRealEngine
{
	struct WindowSettings
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		
		WindowSettings(const std::string& title = "HRealEngine Window", uint32_t width = 1920, uint32_t height = 1080) : Width(width), Height(height), Title(title) {}
	};
	class HREALENGINE_API Window
	{
	public:
		using EventCallbackFn = std::function<void(EventBase&)>;
		virtual ~Window() {}
		virtual void OnUpdate() = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;
		
		static Window* Create(const WindowSettings& settings = WindowSettings());
	};
}

