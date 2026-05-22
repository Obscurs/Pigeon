#pragma once

namespace pig
{
	struct EventComponent
	{
		EventComponent() {};
		EventComponent(const EventComponent&) = default;

		bool m_Dummy = false;
	};
}