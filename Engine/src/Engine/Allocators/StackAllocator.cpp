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
		// If requested block cannot be allocated, return nullptr (alloc, new (std::nothrow) style).
		if (m_Marker + sizeBytes > m_StackSize) return nullptr;
		U8* address = m_StackMemory + m_Marker;
		m_Marker += sizeBytes;
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