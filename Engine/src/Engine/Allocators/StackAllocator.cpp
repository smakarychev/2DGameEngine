#include "enginepch.h"

#include "StackAllocator.h"
#include "MemoryAllocator.h"
#include "Engine/Log.h"

namespace Engine
{
	StackAllocator::StackAllocator(U32 stackSizeBytes) : m_Marker(0), m_StackSize(stackSizeBytes)
	{
		m_StackMemory = reinterpret_cast<U8*>(MemoryAllocator::AllocAligned(stackSizeBytes));
	}

	void* StackAllocator::Alloc(U32 sizeBytes)
	{
		U32 newMarker = m_Marker + sizeBytes;
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newMarker > m_StackSize)
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", sizeBytes, m_StackSize - m_Marker);
			return nullptr;
		}
		U8* address = m_StackMemory + m_Marker;
		m_Marker = newMarker;
		return static_cast<void*>(address);
	}

	void* StackAllocator::AllocAligned(U32 sizeBytes, U16 alignment)
	{
		U16 mask = alignment - 1;
		U32 actualBytes = sizeBytes + mask;

		U32 newMarker = m_Marker + actualBytes;
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (newMarker > m_StackSize) 
		{
			ENGINE_ERROR("Failed to allocate {} bytes: not enough memory ({} bytes)", actualBytes, m_StackSize - m_Marker);
			return nullptr;
		}

		// Align memory address.
		U8* address = MemoryAllocator::AlignPointer(m_StackMemory + m_Marker, alignment);
		m_Marker = newMarker;

		return static_cast<void*>(address);
	}

	U32 StackAllocator::GetMarker() const {	return m_Marker; }

	void StackAllocator::FreeToMarker(U32 marker)
	{
		if (marker <= m_Marker) m_Marker = marker;
		else ENGINE_ERROR("{} is greater than the current stack top ({}).", marker, m_Marker);
	}

	void StackAllocator::Clear()
	{
		m_Marker = 0;
	}

	StackAllocator::~StackAllocator()
	{
		MemoryAllocator::FreeAligned(m_StackMemory);
	}

}