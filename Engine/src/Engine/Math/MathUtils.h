#pragma once

#include "Engine/Core/Types.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <algorithm>

namespace Engine
{
	using namespace Types;
	namespace Math
	{
		inline U32 CLZ(U32 number)
		{
			return __lzcnt(number);
#			ifndef _MSC_VER
			return __builtin_clz(number);
#			endif
		}

		inline U64 CLZ(U64 number)
		{
			return __lzcnt64(number);
#			ifndef _MSC_VER
			return __builtin_clz(number);
#			endif
		}

		// Return a number which is >= `number` and is a power of 2.
		inline U32 CeilToPower2(U32 number)
		{
			return number == 1 ? 1 : (U32)1 << (32 - CLZ(number - 1));
		}

		// Return a number which is >= `number` and is a power of 2.
		inline U64 CeilToPower2(U64 number)
		{
			return number == 1 ? 1 : (U64)1 << (64 - CLZ(number - 1));
		}

		// Returns the ceil(log2(number)).
		inline U32 Log2(U32 number)
		{
			ENGINE_CORE_ASSERT(number != 0, "Log2 of 0 is undefined.");
			return 31 - CLZ(number);
		}

		// Returns the ceil(log2(number)).
		inline U64 Log2(U64 number)
		{
			ENGINE_CORE_ASSERT(number != 0, "Log2 of 0 is undefined.");
			return 63 - CLZ(number);
		}

		// Returns true if `a` and `b` are almost equal.
		inline bool CompareEqual(F32 a, F32 b)
		{
			return std::abs(a - b) <= std::numeric_limits<F32>::epsilon() * std::max(1.0f, std::max(std::abs(a), std::abs(b)));
		}

		// Returns true if `a` and `b` are almost equal.
		inline bool CompareEqual(F64 a, F64 b)
		{
			return std::abs(a - b) <= std::numeric_limits<F64>::epsilon() * std::max(1.0, std::max(std::abs(a), std::abs(b)));
		}

		// Clamps `val` between `min` and `max`.
		template <typename T>
		inline T Clamp(T val, T min, T max)
		{
			return std::clamp<T>(val, min, max);
		}

		template <typename T>
		inline T Abs(T val)
		{
			return std::abs(val);
		}
	}
}
