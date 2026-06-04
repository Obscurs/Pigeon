#pragma once

namespace pg
{
	struct MouseButtonPressedEventComponent
	{
		MouseButtonPressedEventComponent() {};
		MouseButtonPressedEventComponent(const MouseButtonPressedEventComponent&) = default;

		int m_Button = 0;
	};
}