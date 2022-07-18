#pragma once

#include "Engine/Types.h"

namespace Engine
{
	class StackAllocator
	{
	public:
		explicit StackAllocator(U32 stackSizeBytes);
		~StackAllocator();
		
		// Allocate a new block of memory.
		void* Alloc(U32 sizeBytes);

		void* AllocAligned(U32 sizeBytes, U16 alignment);

		template <typename T>
		T* Alloc(U32 count = 1) { return static_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U32 count, U16 alignment) { return static_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		// Get the current stack top.
		U32 GetMarker() const;

		// Rolls the stack back to a previous marker.
		void FreeToMarker(U32 marker);

		// Clears the stack (no memory dealoc).
		void Clear();
	private:
		U8* m_StackMemory;

		// Relative offset.
		U32 m_Marker;

		// Total size of stack.
		U32 m_StackSize;
	};
}