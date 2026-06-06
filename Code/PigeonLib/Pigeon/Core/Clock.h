#pragma once

#include "Pigeon/Core/Timestep.h"

#include <chrono>

namespace pg
{
	class Clock
	{
	public:
		Clock() :
			m_StartingPoint(std::chrono::steady_clock::now())
		{}

		inline const pg::Timestep Elapsed() const noexcept
		{
			return pg::Timestep{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_StartingPoint) };
		}

		inline const pg::Timestep Restart() noexcept
		{
			pg::Timestep elapsed{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_StartingPoint) };
			m_StartingPoint = std::chrono::steady_clock::now();
			return elapsed;
		}

	private:
		std::chrono::steady_clock::time_point m_StartingPoint;
	};
}