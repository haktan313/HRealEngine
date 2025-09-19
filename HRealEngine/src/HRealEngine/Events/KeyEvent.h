
//KeyEvent.h
#pragma once

#include "EventBase.h"

namespace HRealEngine
{
	class HREALENGINE_API KeyEventBase : public EventBase
	{
	public:
		inline int GetKeyCode() const { return keyCodeRef; }
		EVENT_CLASS_CATEGORY(KeyboardEvents | InputEvents)
	protected:
		KeyEventBase(int keyCode) : keyCodeRef(keyCode) {}
		int keyCodeRef;
	};

	class HREALENGINE_API KeyPressedEvent : public KeyEventBase
	{
	public:
		KeyPressedEvent(int keyCode, int repeatCountRef) : KeyEventBase(keyCode), repeatCount(repeatCountRef) {}
		inline int GetRepeatCount() const { return repeatCount; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << GetKeyCode() << " (repeat count: " << repeatCount << ")";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int repeatCount;
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