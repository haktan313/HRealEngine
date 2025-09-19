
//MouseEvent.h
#pragma once

#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API MouseMovedEvent : public EventBase
	{
	public:
		MouseMovedEvent(float x, float y) : mouseX(x), mouseY(y) {}

		inline float GetX() const { return mouseX; }
		inline float GetY() const { return mouseY; }

		
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(MouseEvents | InputEvents)
	private:
		float mouseX, mouseY;
	};

	class HREALENGINE_API MouseScrolledEvent : public EventBase
	{
	public:
		MouseScrolledEvent(float offsetX, float offsetY) : scrollX(offsetX), scrollY(offsetY) {}

		inline float GetOffsetX() const { return scrollX; }
		inline float GetOffsetY() const { return scrollY; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << scrollX << ", " << scrollY;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(MouseEvents | InputEvents)
	private:
		float scrollX, scrollY;
	};

	class HREALENGINE_API MouseButtonEvent : public EventBase
	{
	public:
		inline int GetMouseButton() const { return mouseButton; }
		EVENT_CLASS_CATEGORY(MouseButtonEvents | InputEvents)
	protected:
	protected:
		MouseButtonEvent(int button) : mouseButton(button) {}
		int mouseButton;
	};

	class HREALENGINE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << mouseButton;
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
			ss << "MouseButtonReleasedEvent: " << mouseButton;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}