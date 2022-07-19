#include "enginepch.h"

#include "PoolAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	PoolAllocator::PoolAllocator(U32 typeSizeBytes, U32 count) : m_TypeSizeBytes(typeSizeBytes), m_AllocatedPoolElements(0), m_TotalPoolElements(count)
	{
		ENGINE_ASSERT(typeSizeBytes >= sizeof(void*), "Pool allocator only supports types that are at least as large as {}.", sizeof(void*));
		void* poolMemory = MemoryUtils::AllocAligned(typeSizeBytes * count);
		m_PoolMemory = reinterpret_cast<U8*>(poolMemory);
		m_FreePoolElement = reinterpret_cast<PoolElement*>(poolMemory);

		InitializePool(poolMemory, count);
	}

	void* PoolAllocator::Alloc()
	{
		// If requested block cannot be allocated (the pool is empty), expand pool size.
		if (m_FreePoolElement == nullptr) ExpandPoolSize();
		
		// Take element from the linked list (the pool).
		PoolElement* free = m_FreePoolElement;
		m_FreePoolElement = free->Next;
		m_AllocatedPoolElements++;
		return static_cast<void*>(free);
	}
	
	
	void PoolAllocator::Dealloc(void* memory)
	{
		if (m_AllocatedPoolElements > 0)
		{
			// Return element to the linked list (the pool).
			PoolElement* nextPoolElement = static_cast<PoolElement*>(memory);
			nextPoolElement->Next = m_FreePoolElement;
			m_FreePoolElement = nextPoolElement;
			m_AllocatedPoolElements--;
		}
		else
		{
			ENGINE_ERROR("Cannot deallocate: pull is full.");
		}
	}

	void PoolAllocator::ExpandPoolSize()
	{
		// Allocate additional memory (according to config).
		void* poolExtension = MemoryUtils::AllocAligned(m_TypeSizeBytes * POOL_ALLOCATOR_INCREMENT);
		m_FreePoolElement = reinterpret_cast<PoolElement*>(poolExtension);
		m_AdditionalAllocations.push_back(poolExtension);
		m_TotalPoolElements += POOL_ALLOCATOR_INCREMENT;
		InitializePool(poolExtension, POOL_ALLOCATOR_INCREMENT);
	}

	void PoolAllocator::InitializePool(void* memory, U32 count)
	{
		PoolElement* poolElement = reinterpret_cast<PoolElement*>(memory);
		U8* nextPoolElementAddress = reinterpret_cast<U8*>(memory);
		for (U32 i = 1; i < count; i++)
		{
			nextPoolElementAddress += m_TypeSizeBytes;
			PoolElement* nextPoolElement = reinterpret_cast<PoolElement*>(nextPoolElementAddress);
			poolElement->Next = nextPoolElement;
			poolElement = reinterpret_cast<PoolElement*>(nextPoolElement);
		}
		poolElement->Next = nullptr;
	}

	PoolAllocator::~PoolAllocator()
	{
		MemoryUtils::FreeAligned(m_PoolMemory);
		for (auto& al : m_AdditionalAllocations)
			MemoryUtils::FreeAligned(al);
	}
}
