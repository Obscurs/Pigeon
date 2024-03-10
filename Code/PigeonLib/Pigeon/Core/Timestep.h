#pragma once

namespace pig 
{
	class Timestep
	{
	public:
		Timestep(uint64_t time = 0)
			: m_Time(time)
		{
		}

		operator uint64_t() const { return m_Time; }

		float AsSeconds() const { return m_Time / 1000.f; }
		uint64_t AsMilliseconds() const { return m_Time; }
	private:
		uint64_t m_Time;
	};
}