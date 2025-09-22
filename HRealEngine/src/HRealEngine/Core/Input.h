
#pragma once
#include "Core.h"

namespace HRealEngine
{
	class HREALENGINE_API Input
	{
	public:
		virtual ~Input() = default;
		
		static bool IsKeyPressed(int keyCode) { return s_InstanceOfInput->IsKeyPressedImpl(keyCode);}
		static bool IsMouseButtonPressed(int button) { return s_InstanceOfInput->IsMouseButtonPressedImpl(button); }
		static std::pair<float, float> GetMousePosition() { return s_InstanceOfInput->GetMousePositionImpl(); }
		static float GetMouseX() { return s_InstanceOfInput->GetMouseXImpl(); }
		static float GetMouseY() { return s_InstanceOfInput->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* s_InstanceOfInput;
	};
}