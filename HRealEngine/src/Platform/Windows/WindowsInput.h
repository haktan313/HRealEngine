

#pragma once
#include "HRealEngine/Core/Input.h"

namespace HRealEngine
{
	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(int keyCode) override;
		bool IsMouseButtonPressedImpl(int button) override;
		std::pair<float, float> GetMousePositionImpl() override;
		void SetCursorModeImpl(CursorMode mode) override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};
} 

