#pragma once

#include "Pigeon/Core/EWindowMode.h"

namespace sbx
{
	// The "Window" panel's pending selection, held between frames until Apply. Owned by
	// WindowDebugPanelSystem and seeded once from the live window config.
	struct WindowPanelSelectionSingletonComponent
	{
		WindowPanelSelectionSingletonComponent() = default;
		WindowPanelSelectionSingletonComponent(const WindowPanelSelectionSingletonComponent&) = default;

		int m_PresetIndex = 0;
		pg::EWindowMode m_Mode = pg::EWindowMode::eWindowed;
	};
}
