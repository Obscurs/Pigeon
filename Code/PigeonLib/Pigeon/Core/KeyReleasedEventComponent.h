#pragma once

namespace pig
{
	struct KeyReleasedEventComponent
	{
		KeyReleasedEventComponent() {};
		KeyReleasedEventComponent(const KeyReleasedEventComponent&) = default;

		int m_KeyCode;
	};
}