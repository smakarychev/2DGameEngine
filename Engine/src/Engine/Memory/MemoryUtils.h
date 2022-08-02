#pragma once

#include "Engine/Types.h"
#include "Engine/Core.h"

namespace Engine
{
	class MemoryUtils
	{
	public:
		static uintptr_t AlignAdress(uintptr_t address, U16 alignment);

		template <typename T>
		static T* AlignPointer(T* ptr, U16 alignment)
		{
			const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
			const uintptr_t addressAligned = AlignAdress(address, alignment);
			return reinterpret_cast<T*>(addressAligned);
		}

		static void* AllocAligned(U64 bytes, U16 alignment = alignof(std::max_align_t));
		static void FreeAligned(void* memory);
	};
}