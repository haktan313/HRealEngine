
#pragma once
#include "Core.h"
#include "Entity.h"
#include "glm/vec2.hpp"

namespace HRealEngine
{
	class HREALENGINE_API Input
	{
	public:
		virtual ~Input() = default;
		
		static bool IsKeyPressed(int keyCode) { return s_InstanceOfInput->IsKeyPressedImpl(keyCode);}
		static bool IsMouseButtonPressed(int button) { return s_InstanceOfInput->IsMouseButtonPressedImpl(button); }
		static std::pair<float, float> GetMousePosition() { return s_InstanceOfInput->GetMousePositionImpl(); }
		static glm::vec2 GetViewportMousePos() { return s_ViewportMousePos; }
		static float GetMouseX() { return s_InstanceOfInput->GetMouseXImpl(); }
		static float GetMouseY() { return s_InstanceOfInput->GetMouseYImpl(); }

		static void SetViewportMousePos(float x, float y) { s_ViewportMousePos = { x, y }; }
		static void SetHoveredEntity(Entity* entity) { s_HoveredEntity = entity; }
		static Entity* GetHoveredEntity() { return s_HoveredEntity; }

	protected:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* s_InstanceOfInput;
		static glm::vec2 s_ViewportMousePos;
		static Entity* s_HoveredEntity;
	};
}
