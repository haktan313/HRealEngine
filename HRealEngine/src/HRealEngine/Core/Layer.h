
//Layer.h
#pragma once
#include "Timestep.h"
#include "Core.h"
#include "HRealEngine/Events/AppEvent.h"


namespace HRealEngine
{
	class HREALENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(EventBase& eventRef) {}

		inline const std::string& GetName() const { return debugName; }
	protected:
		std::string debugName;
	};
}

