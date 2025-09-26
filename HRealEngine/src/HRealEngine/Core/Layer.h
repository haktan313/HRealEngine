

#pragma once
#include "Timestep.h"
#include "Core.h"
#include <string> 
#include "HRealEngine/Events/AppEvent.h"


namespace HRealEngine
{
	class HREALENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(EventBase& eventRef) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}

