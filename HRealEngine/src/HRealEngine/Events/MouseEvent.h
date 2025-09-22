
#pragma once
#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API MouseMovedEvent : public EventBase
	{
	public:
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(MouseEvents | InputEvents)
	private:
		float m_MouseX, m_MouseY;
	};

	class HREALENGINE_API MouseScrolledEvent : public EventBase
	{
	public:
		MouseScrolledEvent(float offsetX, float offsetY) : m_ScrollX(offsetX), m_ScrollY(offsetY) {}

		float GetOffsetX() const { return m_ScrollX; }
		float GetOffsetY() const { return m_ScrollY; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_ScrollX << ", " << m_ScrollY;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(MouseEvents | InputEvents)
	private:
		float m_ScrollX, m_ScrollY;
	};

	class HREALENGINE_API MouseButtonEvent : public EventBase
	{
	public:
		int GetMouseButton() const { return m_MouseButton; }
		EVENT_CLASS_CATEGORY(MouseButtonEvents | InputEvents)
	protected:
		MouseButtonEvent(int button) : m_MouseButton(button) {}
		int m_MouseButton;
	};

	class HREALENGINE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_MouseButton;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class HREALENGINE_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
		public:
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_MouseButton;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}