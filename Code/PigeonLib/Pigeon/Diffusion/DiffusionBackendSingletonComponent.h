#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/Diffusion/DiffusionBackend.h"

namespace pg
{
	// Holds the session's diffusion inference backend, created and owned by DiffusionSystem. The
	// resident checkpoint (+ ControlNet) is loaded once; m_LoadAttempted guards that one-time load.
	struct DiffusionBackendSingletonComponent
	{
		pg::S_Ptr<pg::DiffusionBackend> m_Backend;
		bool m_LoadAttempted = false;
	};
}
