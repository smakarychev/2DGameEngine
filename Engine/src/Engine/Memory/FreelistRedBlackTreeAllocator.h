#pragma once

#include "Engine/Types.h"

namespace Engine
{
	// TODO: move to config.
	const U32 RBFREELIST_ALLOCATOR_INCREMENT_BYTES = static_cast<U32>(100_MiB);

	class FreelistRedBlackAllocator
	{
	public:
		explicit FreelistRedBlackAllocator(U32 sizeBytes);
		~FreelistRedBlackAllocator();

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

	private:
		struct FreelistNode;
		struct RedBlackTreeElement;

		void* GetInitializedFreelistHolder(void* memory, U32 sizeBytes);

		// Sets field of FreelistNode structure.
		void* GetInitializedNode(void* memory, U32 sizeBytes);

		void InitializeRBElement(FreelistNode* node, U32 sizeBytes);

		// Allocates extra memory when needed and returns its address.
		void* ExpandFreelist(U32 sizeBytes);

		// Find the address of the node of the sufficient size.
		void* FindFreeNodeAddress(U32 sizeBytes);

		void SplitFreelist(FreelistNode* node, U32 sizeBytes);
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
			U32 SizeAndFlags;
			// We store explicit padding, because 4 byte value is used for size,
			// this will change if we decide to use 8 byte value instead.
			U32 ExplicitPadding;
			RedBlackTreeElement* Parent, * Left, * Right;
			// Size bytes is >= 4, so we use bit 2 for storing color.
			constexpr static U32 PayloadOffset() { return sizeof SizeAndFlags + sizeof ExplicitPadding; }
			constexpr static U32 HeaderSize() { return sizeof Parent + sizeof Left + sizeof Right; }
			bool IsRed() const { return SizeAndFlags & 0b10; }
			void ChangeColor() { SizeAndFlags ^= 0b10; }
			void SetRedColor() { SizeAndFlags |= 0b10; }
			void SetBlackColor() { SizeAndFlags &= ~0b10; }
			U32 SizeBytes() const { return SizeAndFlags & ~(0b11); }
			void SetSizeBytes(U32 size) { SizeAndFlags = size | (SizeAndFlags & 0b11); }
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

			constexpr static U32 HeaderSize() { return sizeof Prev + sizeof Next; }
			constexpr static U32 MinSize() { return HeaderSize() + RedBlackTreeElement::HeaderSize() + RedBlackTreeElement::PayloadOffset(); }
			constexpr static U32 PayloadOffset() { return HeaderSize() + RedBlackTreeElement::PayloadOffset(); }
			RedBlackTreeElement* RBElement() { return reinterpret_cast<RedBlackTreeElement*>(reinterpret_cast<U8*>(this) + HeaderSize()); }

			bool IsNeighbourOf(FreelistNode* other)
			{ 
				return (reinterpret_cast<U8*>(this) + RBElement()->SizeBytes() + PayloadOffset()) == reinterpret_cast<U8*>(other);
			}
		};

		struct FreelistHolder
		{
			FreelistHolder* Next;
			FreelistNode* FirstNode;
			constexpr static U32 HeaderSize() { return sizeof Next + sizeof FirstNode; }
			constexpr static U32 MinSize() { return sizeof HeaderSize() + sizeof FirstNode + FreelistNode::MinSize(); }
		};

		RedBlackTreeElement* m_TreeHead;
		RedBlackTreeElement* m_NullTreeElement;

		FreelistHolder* m_FirstFreelistHolder;

		U8* m_FreelistMemory;
	};
}