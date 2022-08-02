#include "enginepch.h"

#include "BuddyAllocator.h"

#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"
#include "Engine/Math/MathUtils.h"

namespace Engine
{
	BuddyAllocator::BuddyAllocator(U64 sizeBytes, U64 leafSizeBytes) : m_TotalSizeBytes(sizeBytes),
		m_LeafSizeBytes(leafSizeBytes), m_NextBuddyAllocator(nullptr)
	{
		sizeBytes = Math::CeilToPower2(sizeBytes);
		ENGINE_ASSERT((sizeBytes & (sizeBytes - 1)) == 0 && (leafSizeBytes & (leafSizeBytes - 1)) == 0,
			"Buddy allocator size and leafsize have to be the power of 2.");
		m_Levels = static_cast<U32>(Math::Log2(sizeBytes / leafSizeBytes));
		m_Memory = reinterpret_cast<U8*>(MemoryUtils::AllocAligned(sizeBytes, 256));
		m_LevelsMap = std::vector<U64>((U64(1) << (m_Levels + 1)) / 12, 0);

		// Cast 1 to U64 to remove annoying overflow warning.
		m_IsFree = std::vector<bool>(U64(1) << (m_Levels + 1), true);

		// First, we have only one block of level 0.
		m_FreeBlocksLevelList[0] = reinterpret_cast<BuddyAllocatorBlock*>(GetInitializedBlock(m_Memory));
		for (U32 level = 1; level < BUDDY_ALLOCATOR_MAX_LEVELS; level++)
			m_FreeBlocksLevelList[level] = &m_NullNode;

		m_NullNode.Next = m_NullNode.Prev = &m_NullNode;
	}

	void* BuddyAllocator::Alloc(U64 sizeBytes)
	{
		sizeBytes = Math::CeilToPower2(sizeBytes);
		sizeBytes = std::max(sizeBytes, BuddyAllocatorBlock::MinSize());

		// Branch predictor is more than 90% accurate :).
		// If we do not have enough memory, just create new buddy allocator.
		if (sizeBytes > m_TotalSizeBytes)
		{
			if (m_NextBuddyAllocator == nullptr) ExpandBuddyAllocator(sizeBytes);
			return m_NextBuddyAllocator->Alloc(sizeBytes);
		}
		U64 relativeSize = sizeBytes / m_LeafSizeBytes;

		U32 level = static_cast<U32>(m_Levels - Math::Log2(relativeSize));
		BuddyAllocatorBlock* block = reinterpret_cast<BuddyAllocatorBlock*>(AllocBlock(level));
		// If we do not have enough memory, just create new buddy allocator.
		if (block == nullptr) 
		{
			if (m_NextBuddyAllocator == nullptr) ExpandBuddyAllocator(sizeBytes);
			return m_NextBuddyAllocator->Alloc(sizeBytes);
		}
		SetLevel(block, level);
		return static_cast<void*>(block);
	}

	void BuddyAllocator::Dealloc(void* memory)
	{
		// If block wasn't allocated here, delegate it to next allocator.
		if (!DoesBlockBelongsToAllocator(reinterpret_cast<BuddyAllocatorBlock*>(memory)))
		{
			if (m_NextBuddyAllocator != nullptr)
			{
				m_NextBuddyAllocator->Dealloc(memory);
				return;
			}
			ENGINE_ERROR("Buddy allocator: unidentified memory address: {0:x}", reinterpret_cast<U64>(memory));
		}
		U32 level = GetLevel(reinterpret_cast<BuddyAllocatorBlock*>(memory));

		U64 sizeOfBlockInLevel = m_TotalSizeBytes >> level;
		U64 indexInLevel = (reinterpret_cast<U8*>(memory) - m_Memory) / sizeOfBlockInLevel;
		U64 globalBlockIndex = GetGlobalBlockIndex(indexInLevel, level);
		U64 globalBuddyIndex = globalBlockIndex;
		BuddyAllocatorBlock* block = reinterpret_cast<BuddyAllocatorBlock*>(memory);
		BuddyAllocatorBlock* buddy = nullptr;
		// The buddy is to the right.
		if (indexInLevel % 2 == 0)
		{
			buddy = reinterpret_cast<BuddyAllocatorBlock*>(reinterpret_cast<U8*>(block) + sizeOfBlockInLevel);
			globalBuddyIndex++;
		}
		// The buddy is to the left.
		else
		{
			buddy = reinterpret_cast<BuddyAllocatorBlock*>(reinterpret_cast<U8*>(block) - sizeOfBlockInLevel);
			globalBuddyIndex--;
		}

		// Here we add block to the level only if it buddy is not free.
		while (level >= 0)
		{
			if (level != 0 && IsBlockFree(globalBuddyIndex))
			{
				RemoveBlockFromLevel(buddy, level);
				SetBlockFreeStatus(globalBuddyIndex, false);
				if (block > buddy) std::swap(block, buddy);

				U32 levelAbove = level - 1;
				indexInLevel = GetBlockIndexInLevel(block, levelAbove);
				globalBlockIndex = GetGlobalBlockIndex(indexInLevel, levelAbove);
				globalBuddyIndex = globalBlockIndex;
				sizeOfBlockInLevel = m_TotalSizeBytes >> levelAbove;
				// The buddy is to the right.
				if (indexInLevel % 2 == 0)
				{
					buddy = reinterpret_cast<BuddyAllocatorBlock*>(reinterpret_cast<U8*>(block) + sizeOfBlockInLevel);
					globalBuddyIndex++;
				}
				// The buddy is to the left.
				else
				{
					buddy = reinterpret_cast<BuddyAllocatorBlock*>(reinterpret_cast<U8*>(block) - sizeOfBlockInLevel);
					globalBuddyIndex--;
				}
				level = levelAbove;
			}
			else
			{
				AddBlockToLevel(block, level);
				SetBlockFreeStatus(globalBlockIndex, true);
				return;
			}
		}
	}

	void BuddyAllocator::ExpandBuddyAllocator(U64 sizeBytes)
	{
		sizeBytes = std::max(sizeBytes, BUDDY_ALLOCATOR_INCREMENT_BYTES);
		ENGINE_CORE_INFO("Buddy allocator: requesting {} bytes of memory from the system.", sizeBytes);
		m_NextBuddyAllocator = new (MemoryUtils::AllocAligned(sizeof(BuddyAllocator), 256)) BuddyAllocator(sizeBytes, m_LeafSizeBytes);
	}

	void* BuddyAllocator::AllocBlock(U32 level)
	{	
		if (m_FreeBlocksLevelList[level] == &m_NullNode)
		{
			if (level == 0) return nullptr;
			BuddyAllocatorBlock* higherBlock = reinterpret_cast<BuddyAllocatorBlock*>(AllocBlock(level - 1));
			if (higherBlock == nullptr) return nullptr;
			U64 globalIndex = GetGlobalBlockIndex(higherBlock, level - 1);
			SetBlockFreeStatus(globalIndex, false);
			U64 levelSizeBytes = m_TotalSizeBytes >> level;
			BuddyAllocatorBlock* newBlock = higherBlock;

			newBlock->Next = reinterpret_cast<BuddyAllocatorBlock*>(GetInitializedBlock(reinterpret_cast<U8*>(newBlock) + levelSizeBytes));
			newBlock->Next->Prev = newBlock;
			m_FreeBlocksLevelList[level] = newBlock;
		}
		void* blockAddress = m_FreeBlocksLevelList[level];
		m_FreeBlocksLevelList[level] = m_FreeBlocksLevelList[level]->Next;
		SetBlockFreeStatus(reinterpret_cast<BuddyAllocatorBlock*>(blockAddress), level, false);
			
		return blockAddress;
	}

	U64 BuddyAllocator::GetBlockIndexInLevel(BuddyAllocatorBlock* block, U32 level)
	{
		U64 sizeOfBlockInLevel = m_TotalSizeBytes >> level;
		return (reinterpret_cast<U8*>(block) - m_Memory) / sizeOfBlockInLevel;
	}

	U64 BuddyAllocator::GetGlobalBlockIndex(BuddyAllocatorBlock* block, U32 level)
	{
		return GetBlockIndexInLevel(block, level) + (U64(1) << level) - 1;
	}

	U64 BuddyAllocator::GetGlobalBlockIndex(U64 localIndex, U32 level)
	{
		return localIndex + (U64(1) << level) - 1;
	}

	void BuddyAllocator::RemoveBlockFromLevel(BuddyAllocatorBlock* block, U32 level)
	{
		// `block` can be either the head or to the right of the head.
		if (block == m_FreeBlocksLevelList[level])
		{
			m_FreeBlocksLevelList[level] = m_FreeBlocksLevelList[level]->Next;
		}
		else
		{
			block->Prev->Next = block->Next;
			block->Next->Prev = block->Prev;
		}
	}

	void BuddyAllocator::AddBlockToLevel(BuddyAllocatorBlock* block, U32 level)
	{
		block->Next = block->Prev = &m_NullNode;

		if (m_FreeBlocksLevelList[level] == &m_NullNode)
		{
			m_FreeBlocksLevelList[level] = block;
			return;
		}
		block->Next = m_FreeBlocksLevelList[level];
		m_FreeBlocksLevelList[level]->Prev = block;
		m_FreeBlocksLevelList[level] = block;
		return;
	}

	bool BuddyAllocator::IsBlockFree(BuddyAllocatorBlock* block, U32 level)
	{
		return m_IsFree[GetGlobalBlockIndex(block, level)];
	}

	bool BuddyAllocator::IsBlockFree(U64 globalIndex)
	{
		return m_IsFree[globalIndex];
	}

	void BuddyAllocator::SetBlockFreeStatus(BuddyAllocatorBlock* block, U32 level, bool status)
	{
		m_IsFree[GetGlobalBlockIndex(block, level)] = status;
	}

	void BuddyAllocator::SetBlockFreeStatus(U64 globalIndex, bool status)
	{
		m_IsFree[globalIndex] = status;
	}

	bool BuddyAllocator::DoesBlockBelongsToAllocator(BuddyAllocatorBlock* block)
	{
		U8* memory = reinterpret_cast<U8*>(block);
		return (memory >= m_Memory && memory < (m_Memory + m_TotalSizeBytes));
	}

	void BuddyAllocator::SetLevel(BuddyAllocatorBlock* block, U32 level)
	{
		U64 leafLikeBlockAddress = (reinterpret_cast<U8*>(block) - m_Memory) / m_LeafSizeBytes;
		// Should be optimized, since integer division does 2 at the same time.
		U32 indexMajor = static_cast<U32>(leafLikeBlockAddress / 12);
		U32 indexMinor = static_cast<U32>(leafLikeBlockAddress % 12);
		U32 shift = indexMinor * 5;
		U64 mask = ((U64(1) << 5) - 1) << shift;
		m_LevelsMap[indexMajor] &= ~mask;
		m_LevelsMap[indexMajor] |= (static_cast<U64>(level) << shift);
	}

	U32 BuddyAllocator::GetLevel(BuddyAllocatorBlock* block)
	{
		U64 leafLikeBlockAddress = (reinterpret_cast<U8*>(block) - m_Memory) / m_LeafSizeBytes;
		// Should be optimized, since integer division does 2 at the same time.
		U32 indexMajor = static_cast<U32>(leafLikeBlockAddress / 12);
		U32 indexMinor = static_cast<U32>(leafLikeBlockAddress % 12);
		U32 shift = indexMinor * 5;
		U64 mask = ((U64(1) << 5) - 1) << shift;
		return static_cast<U32>((m_LevelsMap[indexMajor] & mask) >> shift);
	}

	void* BuddyAllocator::GetInitializedBlock(void* memory)
	{
		BuddyAllocatorBlock* block = reinterpret_cast<BuddyAllocatorBlock*>(memory);
		block->Next = block->Prev = &m_NullNode;
		return static_cast<void*>(block);
	}


	BuddyAllocator::~BuddyAllocator()
	{
		if (m_NextBuddyAllocator != nullptr) m_NextBuddyAllocator->~BuddyAllocator();
		MemoryUtils::FreeAligned(static_cast<void*>(m_Memory));
	}
}