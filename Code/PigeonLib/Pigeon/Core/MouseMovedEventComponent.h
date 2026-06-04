#pragma once

namespace pg
{
	struct MouseMovedEventComponent
	{
		MouseMovedEventComponent() {};
		MouseMovedEventComponent(const MouseMovedEventComponent&) = default;

		float m_MouseX = 0.f;
		float m_MouseY = 0.f;
	};
}