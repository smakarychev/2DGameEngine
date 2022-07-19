#include "enginepch.h"

#include "MemoryUtils.h"

namespace Engine
{
	uintptr_t MemoryUtils::AlignAdress(uintptr_t address, U16 alignment)
	{
		const U16 mask = alignment - 1;
		ENGINE_ASSERT((alignment & mask) == 0, "Alignment have to be the power of 2.");
		return (address + mask) & ~mask;
	}

	void* MemoryUtils::AllocAligned(U32 bytes, U16 alignment)
	{
		// Extra bytes to ensure proper alignment.
		U32 actualBytes = bytes + alignment;

		// Allocate unaligned block.
		U8* rawMemory = new U8[actualBytes];

		// Calculate aligned memory address. If the offset is 0 (already aligned),
		// shift block by `alignment` to have space to store the offset.
		U8* alignedMemory = AlignPointer(rawMemory, alignment);
		if (alignedMemory == rawMemory)
			alignedMemory += alignment;

		// Calculate  offset and store it in extra memory.
		ptrdiff_t offset = alignedMemory - rawMemory;
		alignedMemory[-1] = static_cast<U8>(offset & 0xFF);

		return alignedMemory;
	}

	void MemoryUtils::FreeAligned(void* memory)
	{
		if (memory)
		{
			// Convert to byte pointer.
			U8* alignedMemory = reinterpret_cast<U8*>(memory);

			// Get the offset
			ptrdiff_t offset = alignedMemory[-1];

			// Since the offset value of 0 is impossible, we use it to indicate an 256 byte offset.
			if (offset == 0) offset = 256;

			// Get the actual memory address and delete it.
			U8* rawMemory = alignedMemory - offset;
			delete[] rawMemory;
		}
	}
}