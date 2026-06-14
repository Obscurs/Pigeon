#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/TextGen/TextGenJob.h"

namespace pg
{
	// Holds the single active (or just-completed) Text Gen Job, owned by TextGenSystem. Null when idle.
	// Only one generation runs at a time; new requests are dropped while a job is in flight.
	struct TextGenJobSingletonComponent
	{
		pg::S_Ptr<pg::TextGenJob> m_ActiveJob;
	};
}
