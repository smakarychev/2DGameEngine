#pragma once

#include "Engine/Core/Types.h"

#include <map>

namespace Engine
{
	using namespace Types;
	// TODO: Move to config.
	static const U64 BUDDY_ALLOCATOR_MAX_LEVELS = 32;
	static const U64 BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES = 16_B;
	static const U64 BUDDY_ALLOCATOR_INCREMENT_BYTES = 128_MiB;

	class BuddyAllocator
	{
	public:
		explicit BuddyAllocator(U64 sizeBytes, U64 leafSizeBytes = BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES);
		~BuddyAllocator();

		// Allocate a new block of memory.
		void* Alloc(U64 sizeBytes);

		// Everything is aligned by default.
		void* AllocAligned(U64 sizeBytes, [[maybe_unused]] U16 alignment) { return Alloc(sizeBytes); }

		template <typename T>
		T* Alloc(U64 count = 1) { return reinterpret_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U64 count, U16 alignment) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocAligned(U64 count = 1) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignof(T))); }

		void Dealloc(void* memory);
		void Dealloc(void* memory, U64 sizeBytes);

		// Recursive version of BelongsLocal.
		bool Belongs(void* memory) const;

		void SetDebugName(const std::string& name) { m_DebugName = name; }
		const std::string& GetDebugName() const { return m_DebugName; }

		// TODO: custom container
		std::vector<U64> GetMemoryBounds() const;
		void SetExpandCallback(void (*callbackFn)()) { m_CallbackFn = callbackFn; }
	private:
		struct BuddyAllocatorBlock;
		
		// Not the greatest name.
		// Checks if memory was allocated here.
		bool BelongsLocal(void* memory) const;

		// Allocate new buddy allocator, if not enough memory.
		void ExpandBuddyAllocator(U64 sizeBytes);
		
		void* GetInitializedBlock(void* memory);

		// Return free block of `level`
		void* AllocBlock(U32 level);

		U64 GetBlockIndexInLevel(BuddyAllocatorBlock* block, U32 level);
		U64 GetGlobalBlockIndex(BuddyAllocatorBlock* block, U32 level);
		U64 GetGlobalBlockIndex(U64 localIndex, U32 level);

		void RemoveBlockFromLevel(BuddyAllocatorBlock* block, U32 level);
		void AddBlockToLevel(BuddyAllocatorBlock* block, U32 level);

		bool IsBlockFree(BuddyAllocatorBlock* block, U32 level);
		bool IsBlockFree(U64 globalIndex);

		void SetBlockFreeStatus(BuddyAllocatorBlock* block, U32 level, bool status);
		void SetBlockFreeStatus(U64 globalIndex, bool status);

		void SetLevel(BuddyAllocatorBlock* block, U32 level);
		U32 GetLevel(BuddyAllocatorBlock* block);

	private:
		struct BuddyAllocatorBlock
		{
			BuddyAllocatorBlock* Next, * Prev;
			constexpr static U64 MinSize() { return sizeof Next + sizeof Prev; }
		};
		
		// Stores pointer to the first free block at each level.
		BuddyAllocatorBlock* m_FreeBlocksLevelList[BUDDY_ALLOCATOR_MAX_LEVELS];

		U64 m_LeafSizeBytes;
		U64 m_TotalSizeBytes;
		U32 m_Levels;

		U8* m_Memory;

		std::vector<bool> m_IsFree;

		std::vector<U64> m_LevelsMap;

		BuddyAllocatorBlock m_NullNode;

		// Extension if ran out of memory.
		BuddyAllocator* m_NextBuddyAllocator;

		std::string m_DebugName;
		
		// This is my favourite line.
		void (*m_CallbackFn)() = [](){};
	};
}