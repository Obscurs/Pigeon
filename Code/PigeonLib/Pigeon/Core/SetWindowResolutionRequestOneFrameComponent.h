#pragma once

#include "Pigeon/Core/EWindowMode.h"

namespace pg
{
	// One-frame request to change the window resolution + display mode at runtime. Emitted by the app,
	// applied (and persisted to the savedata config) by the engine WindowConfigSystem. Must be an engine
	// (pg) type so the engine system can read it.
	struct SetWindowResolutionRequestOneFrameComponent
	{
		SetWindowResolutionRequestOneFrameComponent() = default;
		SetWindowResolutionRequestOneFrameComponent(const SetWindowResolutionRequestOneFrameComponent&) = default;

		unsigned int m_Width = 1280;
		unsigned int m_Height = 720;
		pg::EWindowMode m_Mode = pg::EWindowMode::eWindowed;
	};
}
