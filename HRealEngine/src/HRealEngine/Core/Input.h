
//Input.h
#pragma once
#include "Core.h"
#include "HRpch.h"

namespace HRealEngine
{
	class HREALENGINE_API Input
	{
	public:
		virtual ~Input() = default;
		
		inline static bool IsKeyPressed(int keyCode) { return instanceOfInput->IsKeyPressedImpl(keyCode);}
		inline static bool IsMouseButtonPressed(int button) { return instanceOfInput->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return instanceOfInput->GetMousePositionImpl(); }
		inline static float GetMouseX() { return instanceOfInput->GetMouseXImpl(); }
		inline static float GetMouseY() { return instanceOfInput->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* instanceOfInput;
	};
}