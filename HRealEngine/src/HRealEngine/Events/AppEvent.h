
//AppEvent.h
#pragma once

#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API WindowResizeEvent : public EventBase
	{
	public:
		WindowResizeEvent(unsigned int widthRef, unsigned int heightRef) : width(widthRef), height(heightRef) {}
		inline unsigned int GetWidth() const { return width; }
		inline unsigned int GetHeight() const { return height; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << width << " x " << height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(AppEvents)
	private:
		unsigned int width, height;
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