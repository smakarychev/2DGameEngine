#pragma once

#include "Engine/Types.h"

namespace Engine
{
	// TODO: move to config.
	const U32 FREELIST_ALLOCATOR_INCREMENT_BYTES = static_cast<U32>(8_KiB);

	// Implementation of simple freelist allocator.
	class FreelistAllocator
	{
		enum class FitStrategy
		{
			FirstFit, NextFit, BestFit
		};
	public:
		explicit FreelistAllocator(U32 sizeBytes);
		~FreelistAllocator();


		// Allocates block of memory of size `sizeBytes`
		void* AllocAligned(U32 sizeBytes, U16 alignment);

		void* Alloc(U32 sizeBytes) { return AllocAligned(sizeBytes, alignof(std::max_align_t)); }

		template <typename T>
		T* Alloc(U32 count = 1) { return reinterpret_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U32 count, U16 alignment) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocAligned(U32 count = 1) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignof(T))); }

		void Dealloc(void* memory);

		void SetFitStrategy(FitStrategy strategy);

	private:
		struct FreelistNode;

		// Sets field of FreelistNode structure.
		void* GetInitializedNode(void* memory, U32 size);

		// Allocates extra memory when needed and returns its address.
		void* ExpandFreelist();

		// Splits freelist in two parts: one just enought to contain `sizeBytes` and other is the rest of node.
		void SplitFreelist(FreelistNode* node, U32 sizeBytes);

		void* FindFreeNode(U32 sizeBytes);
		void* FirstFit(U32 sizeBytes);
		void* NextFit(U32 sizeBytes);
		void* BestFit(U32 sizeBytes);

	private:
		struct FreelistNode
		{
			FreelistNode* Next;
			U32 Size;
			bool IsUsed;
			U8* Data;
			constexpr static U32 HeaderSize() { return static_cast<U32>(sizeof Next + sizeof Size + sizeof IsUsed); }
		};


		FreelistNode* m_SearchStart;
		
		FreelistNode* m_FirstNode;
		FreelistNode* m_LastNode;

		FitStrategy m_FitStrategy;

		U8* m_FreelistMemory;

		// TODO: have to change it to use custom memory manager.
		std::vector<void*> m_AdditionalAllocations;
	};
}


