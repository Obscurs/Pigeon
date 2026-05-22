#pragma once

namespace pig
{
	struct MouseScrolledEventComponent
	{
		MouseScrolledEventComponent() {};
		MouseScrolledEventComponent(const MouseScrolledEventComponent&) = default;

		float m_XOffset = 0.f;
		float m_YOffset = 0.f;
	};
}