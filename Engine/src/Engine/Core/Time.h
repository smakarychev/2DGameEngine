#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;
	class Time
	{
	public:
		static F64 Get();
	};
	
	class Timer
	{
	public:
		Timer();
		F64 GetTime() const;
		F64 GetFPS() const;
		void Reset();
	private:
		F64 m_TimeMark;
	};
}
