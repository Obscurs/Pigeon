#pragma once
#include "Pigeon/Events/Event.h"
namespace pig
{
	struct ApplicationEventComponent
	{
		ApplicationEventComponent() {};
		ApplicationEventComponent(const ApplicationEventComponent&) = default;

		pig::Event m_Event;
	};
}

