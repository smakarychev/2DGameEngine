#pragma once

#include "Engine/Types.h"

namespace Engine
{
	// Implementation of double-ended stack allocator.
	class DequeAllocator
	{
	public:
		explicit DequeAllocator(U64 dequeSizeBytes);
		~DequeAllocator();

		// Allocate a new block of memory.

		void* AllocTop(U64 sizeBytes);
		void* AllocBottom(U64 sizeBytes);

		void* AllocTopAligned(U64 sizeBytes, U16 alignment);
		void* AllocBottomAligned(U64 sizeBytes, U16 alignment);

		template <typename T>
		T* AllocTop(U64 count = 1) { return static_cast<T*>(AllocTop(count * sizeof(T))); }
		template <typename T>
		T* AllocBottom(U64 count = 1) { return static_cast<T*>(AllocBottom(count * sizeof(T))); }
		
		template <typename T>
		T* AllocTopAligned(U64 count, U16 alignment) { return static_cast<T*>(AllocTopAligned(count * sizeof(T), alignment)); }
		template <typename T>
		T* AllocBottomAligned(U64 count, U16 alignment) { return static_cast<T*>(AllocBottomAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocTopAligned(U64 count = 1) { return static_cast<T*>(AllocTopAligned(count * sizeof(T), alignof(T))); }
		template <typename T>
		T* AllocBottomAligned(U64 count = 1) { return static_cast<T*>(AllocBottomAligned(count * sizeof(T), alignof(T))); }


		// Get the current stack top.

		U64 GetTopMarker() const;
		U64 GetBottomMarker() const;

		// Rolls the deque back to a previous marker.
		void FreeToMarker(U64 marker);

		// Clears the top part of the deque (no memory dealoc).
		void ClearTop();

		// Clears the bottom part of the deque (no memory dealoc).
		void ClearBottom();

		// Clears the deque (no memory dealoc).
		void Clear();

	private:
		U8* m_DequeMemory;

		// Relative offsets from bottom.
		U64 m_TopMarker, m_BottomMarker;

		// Total size of stack.
		U64 m_DequeSize;
	};
}
