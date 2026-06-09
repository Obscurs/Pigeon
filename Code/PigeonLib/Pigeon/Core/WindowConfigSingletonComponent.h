#pragma once

#include "Pigeon/Core/EWindowMode.h"

namespace pg
{
	// Live window resolution + display mode. Seeded once from the engine config and updated by
	// SetWindowResolutionRequestOneFrameComponent.
	struct WindowConfigSingletonComponent
	{
		WindowConfigSingletonComponent() = default;
		WindowConfigSingletonComponent(const WindowConfigSingletonComponent&) = default;

		unsigned int m_Width = 1280;
		unsigned int m_Height = 720;
		pg::EWindowMode m_Mode = pg::EWindowMode::eWindowed;
	};
}
