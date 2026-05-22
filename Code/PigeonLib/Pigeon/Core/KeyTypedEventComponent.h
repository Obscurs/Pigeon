#pragma once

namespace pig
{
	struct KeyTypedEventComponent
	{
		KeyTypedEventComponent() {};
		KeyTypedEventComponent(const KeyTypedEventComponent&) = default;

		int m_KeyCode;
	};
}