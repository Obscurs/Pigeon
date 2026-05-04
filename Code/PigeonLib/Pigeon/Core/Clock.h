#pragma once

#include <chrono>
#include "Timestep.h"

namespace pig
{
	class Clock
	{
	public:
		Clock() :
			starting_point(std::chrono::steady_clock::now())
		{}

		inline const pig::Timestep Elapsed() const noexcept
		{
			return pig::Timestep{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starting_point) };
		}

		inline const pig::Timestep Restart() noexcept
		{
			pig::Timestep elapsed{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starting_point) };
			starting_point = std::chrono::steady_clock::now();
			return elapsed;
		}

	private:
		std::chrono::steady_clock::time_point starting_point;
	};
}