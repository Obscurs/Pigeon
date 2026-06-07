#pragma once

namespace sbx
{
	// Remembers the last interaction state pushed to the toggle button's image so UIButtonVisualSystem
	// only emits an image update on change. -1 = uninitialised, 0 = default, 1 = hovered, 2 = pressed.
	struct UIButtonVisualStateSingletonComponent
	{
		UIButtonVisualStateSingletonComponent() = default;
		UIButtonVisualStateSingletonComponent(const UIButtonVisualStateSingletonComponent&) = default;

		int m_State = -1;
	};
}
