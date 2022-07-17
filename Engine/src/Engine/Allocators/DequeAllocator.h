#pragma once

#include "Engine/Types.h"

namespace Engine
{
	// Implementation of double-ended stack allocator.
	class DequeAllocator
	{
	public:
		explicit DequeAllocator(U32 dequeSizeBytes);
		~DequeAllocator();

		// Allocate a new block of memory.

		void* AllocTop(U32 sizeBytes);
		void* AllocBottom(U32 sizeBytes);

		template <typename T>
		T* AllocTop(U32 count = 1) { return static_cast<T*>(AllocTop(count * sizeof(T))); }
		template <typename T>
		T* AllocBottom(U32 count = 1) { return static_cast<T*>(AllocBottom(count * sizeof(T))); }
		
		// Get the current stack top.

		U32 GetTopMarker() const;
		U32 GetBottomMarker() const;

		// Rolls the deque back to a previous marker.
		void FreeToMarker(U32 marker);

		// Clears the top part of the deque (no memory dealoc).
		void ClearTop();

		// Clears the bottom part of the deque (no memory dealoc).
		void ClearBottom();

		// Clears the deque (no memory dealoc).
		void Clear();

	private:
		U8* m_DequeMemory;

		// Relative offsets from bottom.
		U32 m_TopMarker, m_BottomMarker;

		// Total size of stack.
		U32 m_DequeSize;
	};
}
