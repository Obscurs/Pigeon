#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	enum class ETextGenJobState
	{
		ePending,
		eRunning,
		eDone,
		eFailed
	};

	// One in-flight (or just-finished) text generation, owned via shared_ptr by
	// TextGenJobSingletonComponent. The background worker writes m_Result and then publishes m_State; the
	// main thread polls m_State and, once eDone, reads m_Result. The destructor joins the worker, so
	// destroying the owning component blocks until the generation finishes.
	struct TextGenJob
	{
		std::atomic<ETextGenJobState> m_State{ ETextGenJobState::ePending };
		pg::UUID m_TargetTextID;
		std::string m_Result;
		std::thread m_Worker;

		TextGenJob() = default;
		~TextGenJob()
		{
			if (m_Worker.joinable())
			{
				m_Worker.join();
			}
		}

		TextGenJob(const TextGenJob&) = delete;
		TextGenJob& operator=(const TextGenJob&) = delete;
	};
}
