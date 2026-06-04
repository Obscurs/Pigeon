#pragma once

namespace pg
{
	struct KeyReleasedEventComponent
	{
		KeyReleasedEventComponent() {};
		KeyReleasedEventComponent(const KeyReleasedEventComponent&) = default;

		int m_KeyCode;
	};
}