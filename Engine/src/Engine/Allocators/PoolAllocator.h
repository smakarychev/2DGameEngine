#pragma once

#include "Engine/Types.h"

namespace Engine
{
	class PoolAllocator
	{
	public:
		PoolAllocator(U32 typeSizeBytes, U32 count);
		~PoolAllocator();

		// Get new element from the pull of free elements.
		void* Alloc();

		template <typename T>
		T* Alloc() { return static_cast<T*>(Alloc()); }

		// Return element to the pull
		void Dealloc();

		// Clears the pull (no memory dealoc).
		void Clear();

	private:
		struct Chunk
		{
			union
			{
				// Store pointer to next element in the same memory.
				Chunk* Next;
				U8* Data;
			};
		};

		U8* m_PoolMemory;
		U32 m_TypeSizeBytes;

		U32 m_AllocatedChunks;
		U32 m_TotalChunks;

		// A pointer to the element to be taken from the pool.
		Chunk* m_FreeChunk;
	};
}
