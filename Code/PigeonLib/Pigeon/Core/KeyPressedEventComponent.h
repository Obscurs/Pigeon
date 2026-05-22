#pragma once

namespace pig
{
	struct KeyPressedEventComponent
	{
		KeyPressedEventComponent() {};
		KeyPressedEventComponent(const KeyPressedEventComponent&) = default;

		int m_KeyCode;
	};
}