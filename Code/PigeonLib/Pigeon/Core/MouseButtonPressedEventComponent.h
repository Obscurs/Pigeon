#pragma once

namespace pig
{
	struct MouseButtonPressedEventComponent
	{
		MouseButtonPressedEventComponent() {};
		MouseButtonPressedEventComponent(const MouseButtonPressedEventComponent&) = default;

		int m_Button = 0;
	};
}