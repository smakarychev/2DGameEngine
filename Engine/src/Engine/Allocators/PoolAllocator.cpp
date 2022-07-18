#include "enginepch.h"

#include "PoolAllocator.h"
#include "MemoryAllocator.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	PoolAllocator::PoolAllocator(U32 typeSizeBytes, U32 count) : m_TypeSizeBytes(typeSizeBytes), m_AllocatedChunks(0), m_TotalChunks(count)
	{
		ENGINE_ASSERT(typeSizeBytes >= sizeof(void*), "Pool allocator only supports types that are at least as large as {}.", sizeof(void*));
		m_PoolMemory = reinterpret_cast<U8*>(MemoryAllocator::AllocAligned(typeSizeBytes * count));
		m_FreeChunk = reinterpret_cast<Chunk*>(m_PoolMemory);

		// Intialize linked list.
		Chunk* chunk = reinterpret_cast<Chunk*>(m_PoolMemory);
		U8* nextChunkAddress = m_PoolMemory;
		for (U32 i = 1; i < count; i++)
		{
			nextChunkAddress += typeSizeBytes;
			Chunk* nextChunk = reinterpret_cast<Chunk*>(nextChunkAddress);
			chunk->Next = nextChunk;
			chunk = reinterpret_cast<Chunk*>(nextChunk);
		}
		// ->Next of last chunk is not set to nullptr because it is unionized with ->Data,
		// instead its ->Data is set to be last bytes of m_PoolMemory.
		chunk->Data = reinterpret_cast<U8*>(chunk);
	}

	void* PoolAllocator::Alloc()
	{
		// If requested block cannot be allocated (the pool is empty), return nullptr (alloc, new (std::nothrow) style).
		if (m_AllocatedChunks == m_TotalChunks) return nullptr;
		
		// Take element from the linked list (the pool).
		Chunk* free = m_FreeChunk;
		m_FreeChunk = free->Next;
		m_AllocatedChunks++;
		return static_cast<void*>(free->Data);
	}
	
	
	void PoolAllocator::Dealloc()
	{
		if (static_cast<void*>(m_FreeChunk) > m_PoolMemory)
		{
			// Return element to the linked list (the pool).
			Chunk* nextChunk = m_FreeChunk;
			m_FreeChunk -= m_TypeSizeBytes;
			m_FreeChunk->Next = nextChunk;
			m_AllocatedChunks--;
		}
		else
		{
			ENGINE_ERROR("Cannot deallocate: pull is full.");
		}
	}

	void PoolAllocator::Clear()
	{
		m_FreeChunk = reinterpret_cast<Chunk*>(m_PoolMemory);
		m_AllocatedChunks = 0;
	}

	PoolAllocator::~PoolAllocator()
	{
		MemoryAllocator::FreeAligned(m_PoolMemory);
	}
}
