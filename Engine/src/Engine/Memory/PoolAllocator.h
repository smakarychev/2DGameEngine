#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;
	// TODO: move to config.
	static constexpr U64 POOL_ALLOCATOR_INCREMENT_ELEMENTS = 8192;
	static constexpr U64 POOL_ALLOCATOR_DEFAULT_COUNT = 16;
	class PoolAllocator
	{
	public:
		PoolAllocator(U64 typeSizeBytes, U64 count = POOL_ALLOCATOR_DEFAULT_COUNT, U64 incrementElements = POOL_ALLOCATOR_INCREMENT_ELEMENTS);
		~PoolAllocator();

		// If vector-like behaviour is desired (warning : invalidation will happen).
		void SetForceContinuous(bool enabled) { m_IsAlwaysContinuous = enabled; }
		// Forbid relocation (expansion).
		void SetNonRelocatable(bool isNonRelocatable) { m_IsNonRelocatable = isNonRelocatable; }		
		
		// Get new element from the pull of free elements.
		void* Alloc();

		void* Alloc([[maybe_unused]] U64 sizeBytes) { return Alloc(); }

		template <typename T>
		T* Alloc() { return static_cast<T*>(Alloc()); }

		// Return element to the pull
		void Dealloc(void* memory);

		void Dealloc(void* memory, [[maybe_unused]] U64 sizeBytes) { Dealloc(memory); }

		// Allocates extra memory when pull is empty and returns its address.
		void* ExpandPool();
	
		// Checks if memory was allocated here.
		bool Belongs(void* memory) const;

		void SetDebugName(const std::string& name) { m_DebugName = name; }
		const std::string& GetDebugName() const { return m_DebugName; }

		// TODO: custom container
		std::vector<U64> GetMemoryBounds() const;
		void SetExpandCallback(void (*callbackFn)()) { m_CallbackFn = callbackFn; }
		U64 GetBaseTypeSize() const { return m_TypeSizeBytes; }
		void* GetPoolHead() const;
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
		U64 m_InitialPoolElements;

		U64 m_IncrementElements;

		// A pointer to the element to be taken from the pool.
		PoolElement* m_FreePoolElement;

		bool m_IsAlwaysContinuous = false;
		bool m_IsNonRelocatable = false;
		
		// TODO: have to change it to use custom memory manager.
		std::vector<void*> m_AdditionalAllocations;

		std::string m_DebugName;

		// This is my favourite line.
		void (*m_CallbackFn)() = [](){};
	};
}
