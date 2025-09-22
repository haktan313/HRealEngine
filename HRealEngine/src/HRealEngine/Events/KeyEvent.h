#pragma once
#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API KeyEventBase : public EventBase
	{
	public:
		int GetKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(KeyboardEvents | InputEvents)
	protected:
		KeyEventBase(int keyCode) : m_KeyCode(keyCode) {}
		int m_KeyCode;
	};

	class HREALENGINE_API KeyPressedEvent : public KeyEventBase
	{
	public:
		KeyPressedEvent(int keyCode, bool bIsRepeate = false) : KeyEventBase(keyCode), m_bIsRepeating(bIsRepeate) {}
		bool IsRepeating() const { return m_bIsRepeating; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << GetKeyCode() << "(repeat = " << m_bIsRepeating << ")";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_bIsRepeating;
	};

	class HREALENGINE_API KeyReleasedEvent : public KeyEventBase
	{
	public:
		KeyReleasedEvent(int keyCode) : KeyEventBase(keyCode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << GetKeyCode();
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	};

	class HREALENGINE_API KeyTypedEvent : public KeyEventBase
	{
	public:
		KeyTypedEvent(int keyCode) : KeyEventBase(keyCode){}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << GetKeyCode();
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyTyped)
	};
}