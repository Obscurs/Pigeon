#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/Timestep.h"
#include "Pigeon/Events/Event.h"

namespace pg 
{
	class PIGEON_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(const Timestep& ts) {}
		virtual bool OnEvent(const Event& event) { return false; }

		virtual void Begin() {};
		virtual void End() {};

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}