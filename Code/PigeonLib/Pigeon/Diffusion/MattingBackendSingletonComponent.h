#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/Diffusion/MattingBackend.h"

namespace pg
{
	// Holds the session's image-matting backend, created and owned by DiffusionSystem. The resident
	// matting model is loaded once; m_LoadAttempted guards that one-time load (ADR 0012). Mirrors
	// DiffusionBackendSingletonComponent.
	struct MattingBackendSingletonComponent
	{
		pg::S_Ptr<pg::MattingBackend> m_Backend;
		bool m_LoadAttempted = false;
	};
}
