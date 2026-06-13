#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/Diffusion/DiffusionJob.h"

namespace pg
{
	// Holds the single active (or just-completed) Diffusion Job, owned by DiffusionSystem. Null when
	// idle. Only one generation runs at a time; new requests are ignored while a job is in flight.
	struct DiffusionJobSingletonComponent
	{
		pg::S_Ptr<pg::DiffusionJob> m_ActiveJob;
	};
}
