#pragma once

namespace pg
{
	struct KeyTypedEventComponent
	{
		KeyTypedEventComponent() {};
		KeyTypedEventComponent(const KeyTypedEventComponent&) = default;

		int m_KeyCode;
	};
}