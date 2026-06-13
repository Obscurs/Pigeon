#pragma once

#include <atomic>
#include <thread>

#include "Pigeon/Core/UUID.h"
#include "Pigeon/Diffusion/Image.h"

namespace pg
{
	enum class EDiffusionJobState
	{
		ePending,
		eRunning,
		eDone,
		eFailed
	};

	// One in-flight (or just-finished) generation, owned via shared_ptr by
	// DiffusionJobSingletonComponent. The background worker writes m_Result and then publishes m_State;
	// the main thread polls m_State and, once eDone, reads m_Result. The destructor joins the worker, so
	// destroying the owning component blocks until the generation finishes.
	struct DiffusionJob
	{
		std::atomic<EDiffusionJobState> m_State{ EDiffusionJobState::ePending };
		pg::UUID m_TargetTextureID;
		pg::Image m_Result;
		std::thread m_Worker;

		DiffusionJob() = default;
		~DiffusionJob()
		{
			if (m_Worker.joinable())
			{
				m_Worker.join();
			}
		}

		DiffusionJob(const DiffusionJob&) = delete;
		DiffusionJob& operator=(const DiffusionJob&) = delete;
	};
}
