

#pragma once
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Core/Layer.h"

namespace HRealEngine
{
	class HREALENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		void Begin();
		void End();
		
		void OnEvent(EventBase& eventRef) override;

		void OnAttach() override;
		void OnDetach() override;
		void SetBlockEvents(bool block) { m_bBlockEvents = block; }
		void SetDarkThemeColors();

	private:
		bool m_bBlockEvents = false;
	};
}


