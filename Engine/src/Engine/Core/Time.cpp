#include "enginepch.h"

#include "Time.h"

namespace Engine
{
	F64 Time::Get()
	{
		return std::chrono::duration<F64, std::ratio<1, 1000>>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
	}
	
	Timer::Timer()
		: m_TimeMark(Time::Get())
	{
	}

	F64 Timer::GetTime() const
	{
		return Time::Get() - m_TimeMark;
	}

	F64 Timer::GetFPS() const
	{
		return 1000.0 / GetTime();
	}

	void Timer::Reset()
	{
		m_TimeMark = Time::Get();
	}

}


