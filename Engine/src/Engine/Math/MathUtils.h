#pragma once

#include "Engine/Types.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace Engine
{
	namespace Math
	{
		inline U32 CLZ(U32 number)
		{
			return __lzcnt(number);
#			ifndef _MSC_VER
			return __builtin_clz(number);
#			endif
		}

		// Return a number which is >= `number` and a power of 2.
		inline U32 CeilToPower2(U32 number)
		{
			return number == 1 ? 1 : (U32)1 << (32 - CLZ(number - 1));
		}

		// Returns the ceil(log2(number));
		inline U32 Log2(U32 number)
		{
			ENGINE_ASSERT(number != 0, "Log2 of 0 is undefined.");
			return 31 - CLZ(number);
		}
	}
}
