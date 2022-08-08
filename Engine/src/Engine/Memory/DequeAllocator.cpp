#include "enginepch.h"
#include "DequeAllocator.h"

#include "DequeAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	DequeAllocator::DequeAllocator(U64 dequeSizeBytes) : m_TopMarker(dequeSizeBytes), m_BottomMarker(0), m_DequeSize(dequeSizeBytes)
	{
		m_DequeMemory = reinterpret_cast<U8*>(MemoryUtils::AllocAligned(dequeSizeBytes));
	}

	void* DequeAllocator::AllocTop(U64 sizeBytes)
	{
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (sizeBytes > m_TopMarker)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U64 newTopMarker = m_TopMarker - sizeBytes;

		// If requested block cannot be allocated, return nullptr
		if (newTopMarker < m_BottomMarker)
		{
			ENGINE_CORE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		m_TopMarker = newTopMarker;
		U8* address = m_DequeMemory + m_TopMarker;
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocBottom(U64 sizeBytes)
	{
		U64 newBottomMarker = m_BottomMarker + sizeBytes;

		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newBottomMarker > m_TopMarker)
		{
			ENGINE_CORE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U8* address = m_DequeMemory + m_BottomMarker;
		m_BottomMarker = newBottomMarker;
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocTopAligned(U64 sizeBytes, U16 alignment)
	{
		U16 mask = alignment - 1;
		U64 actualBytes = sizeBytes + mask;

		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (actualBytes > m_TopMarker)
		{
			ENGINE_CORE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U64 newTopMarker = m_TopMarker - actualBytes;

		// If requested block cannot be allocated, return nullptr
		if (newTopMarker < m_BottomMarker)
		{
			ENGINE_CORE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		m_TopMarker = newTopMarker;
		U8* address = MemoryUtils::AlignPointer(m_DequeMemory + m_TopMarker, alignment);
		return static_cast<void*>(address);
	}

	void* DequeAllocator::AllocBottomAligned(U64 sizeBytes, U16 alignment)
	{
		U16 mask = alignment - 1;
		U64 actualBytes = sizeBytes + mask;

		U64 newBottomMarker = m_BottomMarker + actualBytes;
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newBottomMarker > m_TopMarker)
		{
			ENGINE_CORE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_TopMarker - m_BottomMarker);
			return nullptr;
		}
		U8* address = MemoryUtils::AlignPointer(m_DequeMemory + m_BottomMarker, alignment);
		m_BottomMarker = newBottomMarker;
		return static_cast<void*>(address);
	}

	U64 DequeAllocator::GetTopMarker() const { return m_TopMarker; }

	U64 DequeAllocator::GetBottomMarker() const { return m_BottomMarker; }

	void DequeAllocator::FreeToMarker(U64 marker)
	{
		if (marker <= m_BottomMarker)
			m_BottomMarker = marker;
		else if (marker >= m_TopMarker && marker <= m_DequeSize)
			m_TopMarker = marker;
		else ENGINE_CORE_ERROR("{} is outside of allocated region ([0; {}] - [{}; {}]).", marker, m_BottomMarker, m_TopMarker, m_DequeSize);
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
		MemoryUtils::FreeAligned(m_DequeMemory);
	}
}