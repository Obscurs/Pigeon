#pragma once

namespace pig
{
	struct MouseButtonReleasedEventComponent
	{
		MouseButtonReleasedEventComponent() {};
		MouseButtonReleasedEventComponent(const MouseButtonReleasedEventComponent&) = default;

		int m_Button = 0;
	};
}