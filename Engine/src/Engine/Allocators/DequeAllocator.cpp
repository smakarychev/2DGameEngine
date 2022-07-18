#include "enginepch.h"
#include "DequeAllocator.h"

#include "Engine/Allocators/DequeAllocator.h"
#include "MemoryAllocator.h"
#include "Engine/Log.h"

namespace Engine
{
	DequeAllocator::DequeAllocator(U32 dequeSizeBytes) : m_TopMarker(dequeSizeBytes), m_BottomMarker(0), m_DequeSize(dequeSizeBytes)
	{
		m_DequeMemory = reinterpret_cast<U8*>(MemoryAllocator::AllocAligned(dequeSizeBytes));
	}

	void* DequeAllocator::AllocTop(U32 sizeBytes)
	{
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (sizeBytes > m_TopMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U32 newTopMarker = m_TopMarker - sizeBytes;

		// If requested block cannot be allocated, return nullptr
		if (newTopMarker < m_BottomMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		m_TopMarker = newTopMarker;
		U8* address = m_DequeMemory + m_TopMarker;
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocBottom(U32 sizeBytes)
	{
		U32 newBottomMarker = m_BottomMarker + sizeBytes;

		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newBottomMarker > m_TopMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U8* address = m_DequeMemory + m_BottomMarker;
		m_BottomMarker = newBottomMarker;
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocTopAligned(U32 sizeBytes, U16 alignment)
	{
		U16 mask = alignment - 1;
		U32 actualBytes = sizeBytes + mask;

		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (actualBytes > m_TopMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U32 newTopMarker = m_TopMarker - actualBytes;

		// If requested block cannot be allocated, return nullptr
		if (newTopMarker < m_BottomMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		m_TopMarker = newTopMarker;
		U8* address = MemoryAllocator::AlignPointer(m_DequeMemory + m_TopMarker, alignment);
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocBottomAligned(U32 sizeBytes, U16 alignment)
	{
		U16 mask = alignment - 1;
		U32 actualBytes = sizeBytes + mask;

		U32 newBottomMarker = m_BottomMarker + actualBytes;
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newBottomMarker > m_TopMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U8* address = MemoryAllocator::AlignPointer(m_DequeMemory + m_BottomMarker, alignment);
		m_BottomMarker = newBottomMarker;
		return static_cast<void*>(address);
	}

	U32 DequeAllocator::GetTopMarker() const { return m_TopMarker; }

	U32 DequeAllocator::GetBottomMarker() const { return m_BottomMarker; }

	void DequeAllocator::FreeToMarker(U32 marker)
	{
		if (marker <= m_BottomMarker)
			m_BottomMarker = marker;
		else if (marker >= m_TopMarker && marker <= m_DequeSize)
			m_TopMarker = marker;
		else ENGINE_ERROR("{} is outside of allocated region ([0; {}] - [{}; {}]).", marker, m_BottomMarker, m_TopMarker, m_DequeSize);
	}

	void DequeAllocator::ClearTop()
	{
		m_TopMarker = m_DequeSize;
	}

	void DequeAllocator::ClearBottom()
	{
		m_BottomMarker = 0;
	}

	void DequeAllocator::Clear()
	{
		m_TopMarker = m_DequeSize;
		m_BottomMarker = 0;
	}

	DequeAllocator::~DequeAllocator()
	{
		MemoryAllocator::FreeAligned(m_DequeMemory);
	}
}