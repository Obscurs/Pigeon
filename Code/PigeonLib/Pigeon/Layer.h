#pragma once

#include "Pigeon/Core.h"
#include "Pigeon/Core/Timestep.h"
#include "Pigeon/Events/Event.h"

namespace pigeon 
{
	class PIGEON_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		virtual void Begin() {};
		virtual void End() {};

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}