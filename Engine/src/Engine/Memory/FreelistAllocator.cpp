#include "enginepch.h"

#include "FreelistAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	FreelistAllocator::FreelistAllocator(U32 sizeBytes) : m_FitStrategy(FitStrategy::FirstFit)
	{
		ENGINE_ASSERT(sizeBytes >= FreelistNode::HeaderSize(), "Freelist allocator must be larger.");
		// Allocate the initial chunk of memory, which later will be subdivided into smaller chunks.
		void* freelistMemory = MemoryUtils::AllocAligned(sizeBytes);
		m_FreelistMemory = reinterpret_cast<U8*>(freelistMemory);
		m_FirstNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistMemory, sizeBytes));
		m_LastNode = m_FirstNode;
		m_SearchStart = m_FirstNode;
	}
	
	void* FreelistAllocator::AllocAligned(U32 sizeBytes, U16 alignment)
	{
		U32 actualBytes = sizeBytes + alignment;
		// Find free node with sufficient size. 
		FreelistNode* node = reinterpret_cast<FreelistNode*>(FindFreeNode(actualBytes));

		// Allocate extra space if no freeNode was found.
		if (node == nullptr)
			node = reinterpret_cast<FreelistNode*>(ExpandFreelist());

		// Subdivide memory to smaller chunks.
		SplitFreelist(node, actualBytes);
		node->IsUsed = true;

		// Ensure memory alignment
		U8* alignedMemory = reinterpret_cast<U8*>(MemoryUtils::AlignPointer(node->Data, alignment));
		if (alignedMemory == node->Data)
			alignedMemory += alignment;

		// Calculate  offset and store it in extra memory.
		ptrdiff_t offset = alignedMemory - node->Data;
		alignedMemory[-1] = static_cast<U8>(offset & 0xFF);

		return static_cast<void*>(alignedMemory);
	}

	void FreelistAllocator::Dealloc(void* memory)
	{
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();

		// Get actual address
		U8* alignedMemory = reinterpret_cast<U8*>(memory);
		ptrdiff_t offset = alignedMemory[-1];
		U8* rawMemory = alignedMemory - offset;

		// Get FreelistNode from memory address and mark it as free
		FreelistNode* node = reinterpret_cast<FreelistNode*>(rawMemory - nodeHeaderSize);
		ENGINE_ASSERT(node->IsUsed == true, "node is already free.");
		node->IsUsed = false;
	}

	void FreelistAllocator::SetFitStrategy(FitStrategy strategy)
	{
		m_FitStrategy = strategy;
	}

	void* FreelistAllocator::GetInitializedNode(void* memory, U32 size)
	{
		FreelistNode* node = reinterpret_cast<FreelistNode*>(memory);
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();
		node->Size = size - nodeHeaderSize;
		node->Next = nullptr;
		node->IsUsed = false;
		node->Data = reinterpret_cast<U8*>(node) + nodeHeaderSize;
		return node;
	}

	void* FreelistAllocator::ExpandFreelist()
	{
		// Allocate additional memory (according to config).
		void* freelistExtension = MemoryUtils::AllocAligned(FREELIST_ALLOCATOR_INCREMENT_BYTES);
		m_AdditionalAllocations.push_back(freelistExtension);
		FreelistNode* newLastNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistExtension, FREELIST_ALLOCATOR_INCREMENT_BYTES));
		m_LastNode->Next = newLastNode;
		m_LastNode = newLastNode;
		return freelistExtension;
	}

	void FreelistAllocator::SplitFreelist(FreelistNode* node, U32 sizeBytes)
	{
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();

		U8* newNodeMemoryAddress = reinterpret_cast<U8*>(node) + sizeBytes + nodeHeaderSize;
		U8* alignedNewNodeMemoryAddress = MemoryUtils::AlignPointer(newNodeMemoryAddress, alignof(std::max_align_t));
		U8 offset = static_cast<U8>((alignedNewNodeMemoryAddress - newNodeMemoryAddress) & 0xFF);

		U32 newChunkSize = node->Size - (sizeBytes + offset);
		if (newChunkSize <= nodeHeaderSize) return;
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(alignedNewNodeMemoryAddress, newChunkSize));
		node->Size -= newChunkSize;
		newNode->Next = node->Next;
		node->Next = newNode;
	}

	void* FreelistAllocator::FindFreeNode(U32 sizeBytes)
	{
		switch (m_FitStrategy)
		{
			case Engine::FreelistAllocator::FitStrategy::FirstFit: return FirstFit(sizeBytes);
			case Engine::FreelistAllocator::FitStrategy::NextFit:  return NextFit(sizeBytes);
			case Engine::FreelistAllocator::FitStrategy::BestFit:  return BestFit(sizeBytes);
		}
		return nullptr;
	}

	// Returns first free block of sufficient size, or nullptr if no such block exists.
	void* FreelistAllocator::FirstFit(U32 sizeBytes)
	{
		FreelistNode* node = m_FirstNode;
		while (node)
		{
			if (node->IsUsed || node->Size < sizeBytes)
			{
				node = node->Next;
				continue;
			}
			return node;
		}
		return nullptr;
	}

	// Returns first free block of sufficient size (starting from previously found block), or nullptr if no such block exists.
	void* FreelistAllocator::NextFit(U32 sizeBytes)
	{
		FreelistNode* node = m_SearchStart;
		bool isAllChecked = false;

		while (!isAllChecked)
		{
			if (node->IsUsed || node->Size < sizeBytes)
			{
				node = node->Next ? node->Next : m_FirstNode;
				if (node == m_SearchStart) isAllChecked = true;
					
			}
			m_SearchStart = node->Next ? node->Next : m_FirstNode;
			return node;
		}
		return nullptr;
	}

	// Returns block of smallest size that is greater than `sizeBytes`, or nullptr if no such block exists.
	void* FreelistAllocator::BestFit(U32 sizeBytes)
	{
		FreelistNode* node = m_FirstNode;
		FreelistNode* bestFit = nullptr;
		U32 bestFitSize = std::numeric_limits<U32>::max();

		while (node)
		{
			if (node->IsUsed || node->Size < sizeBytes)
			{
				node = node->Next;
				continue;
			}
			else
			{
				if (node->Size < bestFitSize)
				{
					bestFit = node;
					bestFitSize = bestFit->Size;
				}
			}
		}
		return bestFit;
	}

	FreelistAllocator::~FreelistAllocator()
	{
		MemoryUtils::FreeAligned(static_cast<void*>(m_FreelistMemory));
		for (auto& al : m_AdditionalAllocations)
			MemoryUtils::FreeAligned(al);
	}
}
