#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/TextGen/TextGenBackend.h"

namespace pg
{
	// Holds the session's LLM inference backend, created and owned by TextGenSystem. The resident model
	// is loaded once; m_LoadAttempted guards that one-time load.
	struct TextGenBackendSingletonComponent
	{
		pg::S_Ptr<pg::TextGenBackend> m_Backend;
		bool m_LoadAttempted = false;
	};
}
