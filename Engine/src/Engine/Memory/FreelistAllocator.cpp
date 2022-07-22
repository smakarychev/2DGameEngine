#include "enginepch.h"

#include "FreelistAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	FreelistAllocator::FreelistAllocator(U32 sizeBytes) : m_FitPolicy(FitPolicy::FirstFit)
	{
		ENGINE_ASSERT(sizeBytes >= FreelistNode::HeaderSize(), "Freelist allocator must be larger.");
		// Allocate the initial chunk of memory, which later will be subdivided into smaller chunks.
		void* freelistMemory = MemoryUtils::AllocAligned(sizeBytes);
		m_FreelistMemory = reinterpret_cast<U8*>(freelistMemory);
		m_FirstNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistMemory, sizeBytes));
		m_LastNode = m_FirstNode;
	}
	
	void* FreelistAllocator::AllocAligned(U32 sizeBytes, U16 alignment)
	{
		U32 actualBytes = sizeBytes + alignment;
		// Find free node with sufficient size. 
		void* prevNodeAddress = nullptr;
		FreelistNode* node = reinterpret_cast<FreelistNode*>(FindFreeNodeAddress(actualBytes, &prevNodeAddress));
		FreelistNode* prevNode = reinterpret_cast<FreelistNode*>(prevNodeAddress);

		// Allocate extra space if no freeNode was found.
		if (node == nullptr)
			node = reinterpret_cast<FreelistNode*>(ExpandFreelist());

		// Subdivide memory to smaller chunks and remove used node.
		SplitFreelist(node, actualBytes);
		// If we split the last node, it should change.
		if (m_LastNode->Next) m_LastNode = m_LastNode->Next;
		Remove(node, prevNode);

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

		FreelistNode* node = reinterpret_cast<FreelistNode*>(rawMemory - nodeHeaderSize);
		FreelistNode* prevNode = reinterpret_cast<FreelistNode*>(FindPrevNode(node));
		Insert(node, prevNode);

		MergeFreeList(node, prevNode);
	}

	void FreelistAllocator::SetFitPolicy(FitPolicy strategy)
	{
		m_FitPolicy = strategy;
	}

	void* FreelistAllocator::GetInitializedNode(void* memory, U32 size)
	{
		FreelistNode* node = reinterpret_cast<FreelistNode*>(memory);
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();
		node->Size = size - nodeHeaderSize;
		node->Next = nullptr;
		node->Data = reinterpret_cast<U8*>(node) + nodeHeaderSize;
		return node;
	}

	void* FreelistAllocator::ExpandFreelist()
	{
		// Allocate additional memory (according to config).
		// This allocation can return address which is less than m_FirstNode.
		void* freelistExtension = MemoryUtils::AllocAligned(FREELIST_ALLOCATOR_INCREMENT_BYTES);
		m_AdditionalAllocations.push_back(freelistExtension);
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistExtension, FREELIST_ALLOCATOR_INCREMENT_BYTES));
		if (newNode > m_LastNode)
		{
			m_LastNode->Next = newNode;
			m_LastNode = newNode;
		}
		else if (newNode < m_FirstNode)
		{
			newNode->Next = m_FirstNode;
			m_FirstNode = newNode;
		}
		else
		{
			// It is very safe to assume that this will never happen.
			ENGINE_CORE_FATAL("Impossible allocation in FreelistAllocator.");
		}
		
		return freelistExtension;
	}

	void FreelistAllocator::SplitFreelist(FreelistNode* node, U32 sizeBytes)
	{
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();

		U8* newNodeMemoryAddress = reinterpret_cast<U8*>(node) + sizeBytes + nodeHeaderSize;
		U8* alignedNewNodeMemoryAddress = MemoryUtils::AlignPointer(newNodeMemoryAddress, alignof(void*));
		U8 offset = static_cast<U8>((alignedNewNodeMemoryAddress - newNodeMemoryAddress) & 0xFF);

		U32 newChunkSize = node->Size - (sizeBytes + offset);
		if (newChunkSize <= nodeHeaderSize) return;
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(alignedNewNodeMemoryAddress, newChunkSize));
		node->Size -= newChunkSize;
		newNode->Next = node->Next;
		node->Next = newNode;
	}

	void FreelistAllocator::MergeFreeList(FreelistNode* node, FreelistNode* prevNode)
	{
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();

		// Might seem strange, but node->Next might exist in different region of memory, because of expansion.
		if (node->IsNeighbourOf(node->Next))
		{
			node->Size += node->Next->Size + nodeHeaderSize;
			node->Next = node->Next->Next;
		}
		if (prevNode)
		{
			if (prevNode->IsNeighbourOf(node))
			{
				prevNode->Size += node->Size + nodeHeaderSize;
				prevNode->Next = node->Next;
			}
		}
	}

	void* FreelistAllocator::FindFreeNodeAddress(U32 sizeBytes, void** prevNodeAddress)
	{
		switch (m_FitPolicy)
		{
			case Engine::FreelistAllocator::FitPolicy::FirstFit: return FirstFit(sizeBytes, prevNodeAddress);
			case Engine::FreelistAllocator::FitPolicy::BestFit:  return BestFit(sizeBytes, prevNodeAddress);
		}
		return nullptr;
	}

	// Returns first free block of sufficient size, or nullptr if no such block exists.
	void* FreelistAllocator::FirstFit(U32 sizeBytes, void** prevNodeAddress)
	{
		FreelistNode* node = m_FirstNode;
		FreelistNode* prevNode = nullptr;
		while (node)
		{
			if (node->Size >= sizeBytes) break;
			prevNode = node;
			node = node->Next;
			
		}
		*prevNodeAddress = static_cast<void*>(prevNode);
		return node;
	}

	// Returns block of smallest size that is greater than `sizeBytes`, or nullptr if no such block exists.
	void* FreelistAllocator::BestFit(U32 sizeBytes, void** prevNodeAddress)
	{
		FreelistNode* node = m_FirstNode;
		FreelistNode* prevNode = nullptr;
		FreelistNode* bestFit = nullptr;
		U32 bestFitSize = std::numeric_limits<U32>::max();

		while (node)
		{
			if (node->Size >= sizeBytes)
			{
				if (node->Size < bestFitSize)
				{
					bestFit = node;
					bestFitSize = bestFit->Size;			
					*prevNodeAddress = static_cast<void*>(prevNode);
				}
			}
			prevNode = node;
			node = node->Next;
		}
		if (bestFit == nullptr) *prevNodeAddress = static_cast<void*>(prevNode);
		return bestFit;
	}

	void* FreelistAllocator::FindPrevNode(FreelistNode* node)
	{
		FreelistNode* searchNode = m_FirstNode;
		FreelistNode* prevNode = nullptr;
		while (searchNode && searchNode < node)
		{
			prevNode = searchNode;
			searchNode = searchNode->Next;
		}
		if (searchNode == nullptr) return nullptr;
		return prevNode;
	}

	void FreelistAllocator::Insert(FreelistNode* node, FreelistNode* prevNode)
	{
		if (prevNode == nullptr)
		{
			m_FirstNode = node;
		}
		else
		{
			node->Next = prevNode->Next;
			prevNode->Next = node;
		}
	}

	void FreelistAllocator::Remove(FreelistNode* node, FreelistNode* prevNode)
	{
		if (prevNode == nullptr)
			m_FirstNode = node->Next;
		else prevNode->Next = node->Next;
	}

	FreelistAllocator::~FreelistAllocator()
	{
		MemoryUtils::FreeAligned(static_cast<void*>(m_FreelistMemory));
		for (auto& al : m_AdditionalAllocations)
			MemoryUtils::FreeAligned(al);
	}
}
