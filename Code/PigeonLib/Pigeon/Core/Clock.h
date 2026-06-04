#pragma once

#include <chrono>
#include "Timestep.h"

namespace pg
{
	class Clock
	{
	public:
		Clock() :
			starting_point(std::chrono::steady_clock::now())
		{}

		inline const pg::Timestep Elapsed() const noexcept
		{
			return pg::Timestep{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starting_point) };
		}

		inline const pg::Timestep Restart() noexcept
		{
			pg::Timestep elapsed{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starting_point) };
			starting_point = std::chrono::steady_clock::now();
			return elapsed;
		}

	private:
		std::chrono::steady_clock::time_point starting_point;
	};
}