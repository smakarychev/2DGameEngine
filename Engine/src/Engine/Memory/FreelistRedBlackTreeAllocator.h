#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;
	// TODO: move to config.
	static const U64 RBFREELIST_ALLOCATOR_INCREMENT_BYTES = 100_MiB;

	class FreelistRedBlackAllocator
	{
	public:
		explicit FreelistRedBlackAllocator(U64 sizeBytes);
		~FreelistRedBlackAllocator();

		// Allocates block of memory of size `sizeBytes`
		void* AllocAligned(U64 sizeBytes, U16 alignment);

		void* Alloc(U64 sizeBytes) { return AllocAligned(sizeBytes, alignof(std::max_align_t)); }

		template <typename T>
		T* Alloc(U64 count = 1) { return reinterpret_cast<T*>(Alloc(count * sizeof(T))); }

		template <typename T>
		T* AllocAligned(U64 count, U16 alignment) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignment)); }

		template <typename T>
		T* AllocAligned(U64 count = 1) { return reinterpret_cast<T*>(AllocAligned(count * sizeof(T), alignof(T))); }

		void Dealloc(void* memory);
		void Dealloc(void* memory, [[maybe_unused]] U64 sizeBytes) { Dealloc(memory); }

		// Checks if memory was allocated here.
		bool Belongs(void* memory);

		void SetDebugName(const std::string& name) { m_DebugName = name; }
		const std::string& GetDebugName() const { return m_DebugName; }

		// TODO: custom container
		std::vector<U64> GetMemoryBounds() const;
		void SetExpandCallback(void (*callbackFn)()) { m_CallbackFn = callbackFn; }
	private:
		struct FreelistNode;
		struct RedBlackTreeElement;

		void* GetInitializedFreelistHolder(void* memory, U64 sizeBytes);

		// Sets field of FreelistNode structure.
		void* GetInitializedNode(void* memory, U64 sizeBytes);

		void InitializeRBElement(FreelistNode* node, U64 sizeBytes);

		// Allocates extra memory when needed and returns its address.
		void* ExpandFreelist(U64 sizeBytes);

		// Find the address of the node of the sufficient size.
		void* FindFreeNodeAddress(U64 sizeBytes);

		void SplitFreelist(FreelistNode* node, U64 sizeBytes);
		void MergeFreelist(FreelistNode* node);


		// Methods for handling Red Black Tree.

		void InsertRB(RedBlackTreeElement* element);
		void DeleteRB(RedBlackTreeElement* element);
		void TransplantRB(RedBlackTreeElement* U, RedBlackTreeElement* V);
		void* MinimumRB(RedBlackTreeElement* element);
		void FixTreeInsert(RedBlackTreeElement* element);
		void FixTreeDelete(RedBlackTreeElement* element);
		void LeftRotation(RedBlackTreeElement* element);
		void RightRotation(RedBlackTreeElement* element);
		void ClearRB(RedBlackTreeElement* element);

	private:
		struct RedBlackTreeElement
		{
			U64 SizeAndFlags;
			RedBlackTreeElement* Parent, * Left, * Right;
			// Size bytes is >= 4, so we use bit 2 for storing color.
			constexpr static U64 PayloadOffset() { return sizeof SizeAndFlags; }
			constexpr static U64 HeaderSize() { return sizeof Parent + sizeof Left + sizeof Right; }
			bool IsRed() const { return SizeAndFlags & 0b10; }
			void ChangeColor() { SizeAndFlags ^= 0b10; }
			void SetRedColor() { SizeAndFlags |= 0b10; }
			void SetBlackColor() { SizeAndFlags &= ~0b10; }
			U64 SizeBytes() const { return SizeAndFlags & ~(0b11); }
			void SetSizeBytes(U64 size) { SizeAndFlags = size | (SizeAndFlags & 0b11); }
		};

		// The node of allocator, we store rb tree only for free elements.
		struct FreelistNode
		{
			FreelistNode* Prev, * Next;
			// Size bytes is >= 4, so we use bit 1 for storing state.
			bool IsFree() { return RBElement()->SizeAndFlags & 0b01;  }
			void ChangeState() { RBElement()->SizeAndFlags ^= 0b01; }
			void SetFree() { RBElement()->SizeAndFlags |= 0b01; }
			void SetUsed() { RBElement()->SizeAndFlags &= ~0b01; }

			constexpr static U64 HeaderSize() { return sizeof Prev + sizeof Next; }
			constexpr static U64 MinSize() { return HeaderSize() + RedBlackTreeElement::HeaderSize() + RedBlackTreeElement::PayloadOffset(); }
			constexpr static U64 PayloadOffset() { return HeaderSize() + RedBlackTreeElement::PayloadOffset(); }
			RedBlackTreeElement* RBElement() { return reinterpret_cast<RedBlackTreeElement*>(reinterpret_cast<U8*>(this) + HeaderSize()); }

			bool IsNeighbourOf(FreelistNode* other)
			{ 
				return (reinterpret_cast<U8*>(this) + RBElement()->SizeBytes() + PayloadOffset()) == reinterpret_cast<U8*>(other);
			}
		};

		// Struct for each memory request from os (in ExpandFreelist()).
		struct FreelistHolder
		{
			U64 SizeBytes;
			FreelistHolder* Next;
			FreelistNode* FirstNode;
			constexpr static U64 HeaderSize() { return sizeof SizeBytes + sizeof Next + sizeof FirstNode; }
			constexpr static U64 MinSize() { return sizeof HeaderSize() + sizeof FirstNode + FreelistNode::MinSize(); }
		};

		RedBlackTreeElement* m_TreeHead;
		RedBlackTreeElement* m_NullTreeElement;

		FreelistHolder* m_FirstFreelistHolder;

		std::string m_DebugName;

		// This is my favourite line.
		void (*m_CallbackFn)() = [](){};
	};
}