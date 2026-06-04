#pragma once

namespace pg
{
	struct KeyPressedEventComponent
	{
		KeyPressedEventComponent() {};
		KeyPressedEventComponent(const KeyPressedEventComponent&) = default;

		int m_KeyCode;
	};
}