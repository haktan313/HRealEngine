
#pragma once

#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API WindowResizeEvent : public EventBase
	{
	public:
		WindowResizeEvent(unsigned int widthRef, unsigned int heightRef) : m_Width(widthRef), m_Height(heightRef) {}
		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << " x " << m_Height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(AppEvents)
	private:
		unsigned int m_Width, m_Height;
	};

	class HREALENGINE_API WindowCloseEvent : public EventBase
	{
	public:
		WindowCloseEvent() {}
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppTickEvent : public EventBase
	{
	public:
		AppTickEvent() {}
		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppUpdateEvent : public EventBase
	{
	public:
		AppUpdateEvent() {}
		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppRenderEvent : public EventBase
	{
	public:
		AppRenderEvent() {}
		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
}