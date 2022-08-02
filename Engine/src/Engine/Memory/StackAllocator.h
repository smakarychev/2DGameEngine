#pragma once

#include "Engine/Types.h"

namespace Engine
{
	class StackAllocator
	{
	public:
		explicit StackAllocator(U64 stackSizeBytes);
		~StackAllocator();
		
		// Allocate a new block of memory.
		void* Alloc(U64 sizeBytes);

		void* AllocAligned(U64 sizeBytes, U16 alignment);

		template <typename T>
		T* Alloc(U64 count = 1) { return static_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U64 count, U16 alignment) { return static_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocAligned(U64 count = 1) { return static_cast<T*>(AllocAligned(count * sizeof(T), alignof(T))); }

		// Get the current stack top.
		U64 GetMarker() const;

		// Rolls the stack back to a previous marker.
		void FreeToMarker(U64 marker);

		// Clears the stack (no memory dealoc).
		void Clear();
	private:
		U8* m_StackMemory;

		// Relative offset.
		U64 m_Marker;

		// Total size of stack.
		U64 m_StackSize;
	};
}