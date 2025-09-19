
//ImGuiLayer.h
#pragma once
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Core/Layer.h"

namespace HRealEngine
{
	class HREALENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void Begin();
		void End();

		virtual void OnEvent(EventBase& eventRef) override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void SetBlockEvents(bool block) { bBlockEvents = block; }
		void SetDarkThemeColors();

	private:
		bool bBlockEvents = false;
		float timeForDelta = 0.0f;
	};
}


