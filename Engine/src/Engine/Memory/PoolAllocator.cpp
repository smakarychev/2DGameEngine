#include "enginepch.h"

#include "PoolAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	PoolAllocator::PoolAllocator(U64 typeSizeBytes, U64 count, U64 incrementElements) :
		m_TypeSizeBytes(typeSizeBytes), m_AllocatedPoolElements(0),
		m_TotalPoolElements(count), m_InitialPoolElements(count),
		m_IncrementElements(incrementElements),
		m_DebugName("Pool Allocator")
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
		if (m_FreePoolElement == nullptr) m_FreePoolElement = reinterpret_cast<PoolElement*>(ExpandPool());
		
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
			ENGINE_ERROR("{}: cannot deallocate: pull is full.", m_DebugName);
		}
	}

	void* PoolAllocator::ExpandPool()
	{
		// Allocate additional memory (according to config).
		void* poolExtension = MemoryUtils::AllocAligned(m_TypeSizeBytes * m_IncrementElements);
		ENGINE_INFO("{}: requesting {} bytes of memory from the system.", m_DebugName, m_TypeSizeBytes * m_IncrementElements);
		m_AdditionalAllocations.push_back(poolExtension);
		m_TotalPoolElements += m_IncrementElements;
		InitializePool(poolExtension, m_IncrementElements);

		// Callback is defined in memory manager.
		m_CallbackFn();

		return poolExtension;
	}

	bool PoolAllocator::Belongs(void* memory) const
	{
		U8* address = reinterpret_cast<U8*>(memory);
		if (address >= m_PoolMemory && address < m_PoolMemory + m_InitialPoolElements * m_TypeSizeBytes) return true;
		for (const auto& al : m_AdditionalAllocations)
		{
			U8* alAddress = reinterpret_cast<U8*>(al);
			if (address >= alAddress && address < alAddress + m_IncrementElements * m_TypeSizeBytes) return true;
		}
		return false;
	}

	std::vector<U64> PoolAllocator::GetMemoryBounds() const
	{
		std::vector<U64> bounds;
		bounds.push_back(reinterpret_cast<U64>(m_PoolMemory));
		bounds.push_back(reinterpret_cast<U64>(m_PoolMemory + m_InitialPoolElements * m_TypeSizeBytes));
		for (const auto& al : m_AdditionalAllocations)
		{
			U8* alAddress = reinterpret_cast<U8*>(al);
			bounds.push_back(reinterpret_cast<U64>(alAddress));
			bounds.push_back(reinterpret_cast<U64>(alAddress + m_IncrementElements * m_TypeSizeBytes));
		}
		return bounds;	
	}

	void PoolAllocator::InitializePool(void* memory, U64 count)
	{
		PoolElement* poolElement = reinterpret_cast<PoolElement*>(memory);
		U8* nextPoolElementAddress = reinterpret_cast<U8*>(memory);
		for (U64 i = 1; i < count; i++)
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
