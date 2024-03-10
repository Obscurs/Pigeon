#pragma once

namespace pig 
{
	class Timestep
	{
	public:
		Timestep(uint64_t time = 0)
			: m_Time(time)
		{}

		Timestep(std::chrono::milliseconds duration) 
			: m_Time(static_cast<uint64_t>(duration.count()))
		{}

		operator uint64_t() const { return m_Time; }

		float AsSeconds() const { return m_Time / 1000.f; }
		uint64_t AsMilliseconds() const { return m_Time; }

	private:
		uint64_t m_Time;
	};
}