#pragma once

namespace pig 
{
	class Timestep
	{
	public:
		explicit Timestep(uint64_t time = 0) noexcept
			: m_Time(time)
		{}

		explicit Timestep(std::chrono::milliseconds duration) noexcept
			: m_Time(static_cast<uint64_t>(duration.count()))
		{}

		operator uint64_t() const noexcept { return m_Time; }

		float AsMinutes() const noexcept { return m_Time / 1000.f / 60.f; }
		float AsSeconds() const noexcept { return m_Time / 1000.f; }
		uint64_t AsMilliseconds() const noexcept { return m_Time; }

	private:
		uint64_t m_Time;
	};
}