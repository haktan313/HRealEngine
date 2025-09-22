
#pragma once

#include "HRealEngine/Core/Core.h"
#include <sstream>
#include <string>

namespace HRealEngine
{
	enum class EventTypes
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};
	enum EventCategory
	{
		None = 0,
		AppEvents = BIT(0),
		InputEvents = BIT(1),
		KeyboardEvents = BIT(2),
		MouseEvents = BIT(3),
		MouseButtonEvents = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventTypes GetEventStaticType() { return EventTypes::type; } \
                                virtual EventTypes GetType() const override { return GetEventStaticType(); } \
                                virtual const char* GetName() const override { return #type; } \

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class HREALENGINE_API EventBase
	{
		friend class EventDispatcher;
	public:
		virtual ~EventBase() = default;
		
		virtual EventTypes GetType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }

		bool m_bHandled = false;
	};

	class EventDispatcher
	{
		//type alias
		template<typename comingEvent> using EventFunction = std::function<bool(comingEvent&)>;
	public:
		EventDispatcher(EventBase& event) : m_Event(event) {}
		//template
		template<typename comingEvent>
		bool Dispatch(EventFunction<comingEvent> /*std::function<bool(WindowCloseEvent&)>*/ func)
		{
			if (m_Event.GetType() == comingEvent::GetEventStaticType())
			{
				m_Event.m_bHandled = func(*(comingEvent*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		EventBase& m_Event;
	};
	inline std::ostream& operator<<(std::ostream& os, const EventBase& e)
	{
		return os << e.ToString();
	}
}