#pragma once

namespace pg
{
	struct EventComponent
	{
		EventComponent() {};
		EventComponent(const EventComponent&) = default;

		bool m_Dummy = false;
	};
}