#pragma once

#include "Engine/Types.h"

namespace Engine
{
	// TODO: move to config.
	static const U64 POOL_ALLOCATOR_INCREMENT_ELEMENTS = 32;
	class PoolAllocator
	{
	public:
		PoolAllocator(U64 typeSizeBytes, U64 count);
		~PoolAllocator();

		// Get new element from the pull of free elements.
		void* Alloc();

		template <typename T>
		T* Alloc() { return static_cast<T*>(Alloc()); }

		// Return element to the pull
		void Dealloc(void* memory);

		// Allocates extra memory when pull is empty and returns its address.
		void* ExpandPool();
	
	private:
		void InitializePool(void* memory, U64 count);
	private:
		struct PoolElement
		{
			// This represents data and pointer to next at the same time.
			PoolElement* Next;
		};

		U8* m_PoolMemory;
		U64 m_TypeSizeBytes;

		U64 m_AllocatedPoolElements;
		U64 m_TotalPoolElements;

		// A pointer to the element to be taken from the pool.
		PoolElement* m_FreePoolElement;

		// TODO: have to change it to use custom memory manager.
		std::vector<void*> m_AdditionalAllocations;
	};
}
