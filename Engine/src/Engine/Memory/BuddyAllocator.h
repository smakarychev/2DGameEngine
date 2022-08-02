#pragma once

#include "Engine/Types.h"

#include <map>

namespace Engine
{
	// TODO: Move to config.
	static const U32 BUDDY_ALLOCATOR_MAX_LEVELS = 32;
	static const U32 BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES = U32(16_B);
	static const U32 BUDDY_ALLOCATOR_INCREMENT_BYTES = static_cast<U32>(128_MiB);

	class BuddyAllocator
	{
	public:
		explicit BuddyAllocator(U32 sizeBytes, U32 leafSizeBytes = BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES);
		~BuddyAllocator();

		void* Alloc(U32 sizeBytes);

		void* AllocAligned(U32 sizeBytes, U16 alignment) { return Alloc(sizeBytes); }

		template <typename T>
		T* Alloc(U32 count = 1) { return reinterpret_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U32 count, U16 alignment) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocAligned(U32 count = 1) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignof(T))); }

		void Dealloc(void* memory);

	private:
		void ExpandBuddyAllocator(U32 sizeBytes);

		struct BuddyAllocatorBlock;

		void* GetInitializedBlock(void* memory, U32 sizeBytes);

		void* AllocBlock(U32 level);

		U32 GetBlockIndexInLevel(BuddyAllocatorBlock* block, U32 level);
		U32 GetGlobalBlockIndex(BuddyAllocatorBlock* block, U32 level);
		U32 GetGlobalBlockIndex(U32 localIndex, U32 level);

		void RemoveBlockFromLevel(BuddyAllocatorBlock* block, U32 level);
		void AddBlockToLevel(BuddyAllocatorBlock* block, U32 level);

		bool IsBlockFree(BuddyAllocatorBlock* block, U32 level);
		bool IsBlockFree(U32 globalIndex);

		void SetBlockFreeStatus(BuddyAllocatorBlock* block, U32 level, bool status);
		void SetBlockFreeStatus(U32 globalIndex, bool status);

		// Not the greatest name.
		// Checks if block was allocated here.
		bool DoesBlockBelongsToAllocator(BuddyAllocatorBlock* block);

		void SetLevel(BuddyAllocatorBlock* block, U32 level);
		U32 GetLevel(BuddyAllocatorBlock* block);

	private:
		struct BuddyAllocatorBlock
		{
			BuddyAllocatorBlock* Next, * Prev;
			constexpr static U32 MinSize() { return sizeof Next + sizeof Prev; }
		};
		
		// Stores pointer to the first free block at each level.
		BuddyAllocatorBlock* m_FreeBlocksLevelList[BUDDY_ALLOCATOR_MAX_LEVELS];

		U32 m_LeafSizeBytes;
		U32 m_TotalSizeBytes;
		U32 m_Levels;

		U8* m_Memory;

		std::vector<bool> m_IsFree;

		std::vector<U64> m_LevelsMap;

		BuddyAllocatorBlock m_NullNode;

		// Extension if ran out of memory.
		BuddyAllocator* m_NextBuddyAllocator;
	};
}