#pragma once
#include "Pigeon/Events/Event.h"
namespace pg
{
	struct ApplicationEventComponent
	{
		ApplicationEventComponent() {};
		ApplicationEventComponent(const ApplicationEventComponent&) = default;

		pg::Event m_Event;
	};
}

