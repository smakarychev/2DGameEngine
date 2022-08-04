#include "enginepch.h"

#include "FreelistRedBlackTreeAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	FreelistRedBlackAllocator::FreelistRedBlackAllocator(U64 sizeBytes) :
		m_DebugName("Freelist allocator")
	{
		ENGINE_ASSERT(sizeBytes >= FreelistHolder::MinSize(), "Freelist allocator must be larger.");
		void* freelistMemory = MemoryUtils::AllocAligned(sizeBytes);

		m_NullTreeElement = reinterpret_cast<RedBlackTreeElement*>(MemoryUtils::AllocAligned(sizeof(RedBlackTreeElement), alignof(RedBlackTreeElement)));
		m_NullTreeElement->Left = m_NullTreeElement->Right = m_NullTreeElement;
		m_NullTreeElement->Parent = nullptr;
		m_NullTreeElement->SizeAndFlags = 0;

		m_FirstFreelistHolder = reinterpret_cast<FreelistHolder*>(GetInitializedFreelistHolder(freelistMemory, sizeBytes));
		m_TreeHead = m_NullTreeElement;
		InsertRB(m_FirstFreelistHolder->FirstNode->RBElement());
	}

	void* FreelistRedBlackAllocator::AllocAligned(U64 sizeBytes, U16 alignment)
	{
		constexpr static U64 payloadOffset = FreelistNode::PayloadOffset();
		constexpr static U64 treeElementHeader = RedBlackTreeElement::HeaderSize();

		// Since at some point we want to "return" node, 
		// we have to make sure, that essencial tree data will fit.
		U64 actualBytes = std::max(sizeBytes, treeElementHeader) + alignment;
		// Find free node with sufficient size. 
		FreelistNode* node = reinterpret_cast<FreelistNode*>(FindFreeNodeAddress(actualBytes));

		// Allocate extra space if no freeNode was found.
		if (node == nullptr)
			node = reinterpret_cast<FreelistNode*>(ExpandFreelist(actualBytes));

		// Subdivide memory to smaller chunks and mark used node.
		SplitFreelist(node, actualBytes);
		node->SetUsed();
		DeleteRB(node->RBElement());

		// Ensure memory alignment
		U8* data = reinterpret_cast<U8*>(node) + payloadOffset;
		U8* alignedMemory = reinterpret_cast<U8*>(MemoryUtils::AlignPointer(data, alignment));
		if (alignedMemory == data)
			alignedMemory += alignment;

		// Calculate  offset and store it in extra memory.
		ptrdiff_t offset = alignedMemory - data;
		alignedMemory[-1] = static_cast<U8>(offset & 0xFF);

		return static_cast<void*>(alignedMemory);
	}

	void FreelistRedBlackAllocator::Dealloc(void* memory)
	{
		constexpr static U64 payloadOffset = FreelistNode::PayloadOffset();

		// Get actual address
		U8* alignedMemory = reinterpret_cast<U8*>(memory);
		ptrdiff_t offset = alignedMemory[-1];
		U8* rawMemory = alignedMemory - offset;

		FreelistNode* node = reinterpret_cast<FreelistNode*>(rawMemory - payloadOffset);
		node->SetFree();
		// Try merge free neighbour nodes.
		MergeFreelist(node);
	}

	bool FreelistRedBlackAllocator::Belongs(void* memory)
	{
		U8* address = reinterpret_cast<U8*>(memory);
		FreelistHolder* holder = m_FirstFreelistHolder;
		U8* holderAddress = reinterpret_cast<U8*>(holder);
		if (address >= holderAddress && address < holderAddress + holder->SizeBytes) return true;
		while (holder->Next)
		{
			holder = holder->Next;
			holderAddress = reinterpret_cast<U8*>(holder);
			if (address >= holderAddress && address < holderAddress + holder->SizeBytes) return true;
		}
		return false;
	}
	
	std::vector<U64> FreelistRedBlackAllocator::GetMemoryBounds() const
	{

		std::vector<U64> bounds;

		FreelistHolder* holder = m_FirstFreelistHolder;
		U8* holderAddress = reinterpret_cast<U8*>(holder);

		bounds.push_back(reinterpret_cast<U64>(holderAddress));
		bounds.push_back(reinterpret_cast<U64>(holderAddress + holder->SizeBytes));
		while (holder->Next)
		{
			holder = holder->Next;
			holderAddress = reinterpret_cast<U8*>(holder);
			bounds.push_back(reinterpret_cast<U64>(holderAddress));
			bounds.push_back(reinterpret_cast<U64>(holderAddress + holder->SizeBytes));
		}
		return bounds;
	}

	void* FreelistRedBlackAllocator::GetInitializedFreelistHolder(void* memory, U64 sizeBytes)
	{
		constexpr static U64 holderHeader = FreelistHolder::HeaderSize();
		FreelistHolder* holder = reinterpret_cast<FreelistHolder*>(memory);
		holder->SizeBytes = sizeBytes;
		holder->Next = nullptr;

		U8* holderNodeAddress = static_cast<U8*>(memory) + holderHeader;
		U64 holderNodeSize = sizeBytes - holderHeader;
		FreelistNode* holderNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(holderNodeAddress, holderNodeSize));
		holder->FirstNode = holderNode;

		return static_cast<void*>(holder);
	}

	void* FreelistRedBlackAllocator::GetInitializedNode(void* memory, U64 sizeBytes)
	{
		FreelistNode* node = reinterpret_cast<FreelistNode*>(memory);
		node->Next = node->Prev = nullptr;
		InitializeRBElement(node, sizeBytes);
		node->SetFree(); 
		return node;
	}

	void FreelistRedBlackAllocator::InitializeRBElement(FreelistNode* node, U64 sizeBytes)
	{
		constexpr static U64 payloadOffset = FreelistNode::PayloadOffset();
		RedBlackTreeElement* rbElement = reinterpret_cast<RedBlackTreeElement*>(node->RBElement());
		rbElement->SizeAndFlags = sizeBytes - payloadOffset;
		rbElement->Parent = rbElement->Left = rbElement->Right = m_NullTreeElement;
		rbElement->SetRedColor();
	}

	void* FreelistRedBlackAllocator::ExpandFreelist(U64 sizeBytes)
	{
		// Allocate additional memory (according to config).
		// This allocation can return address which is less than m_FirstNode.
		sizeBytes = std::max(sizeBytes, RBFREELIST_ALLOCATOR_INCREMENT_BYTES);
		sizeBytes += FreelistHolder::HeaderSize() + FreelistNode::PayloadOffset() + static_cast<U64>(alignof(void*));

		ENGINE_CORE_INFO("{}: requesting {} bytes of memory from the system.", m_DebugName, sizeBytes);

		void* freelistExtension = MemoryUtils::AllocAligned(sizeBytes);

		FreelistHolder* newHolder = reinterpret_cast<FreelistHolder*>(GetInitializedFreelistHolder(freelistExtension, sizeBytes));
		newHolder->Next = m_FirstFreelistHolder;
		newHolder->FirstNode->Next = m_FirstFreelistHolder->FirstNode;
		m_FirstFreelistHolder->FirstNode->Prev = newHolder->FirstNode;
		m_FirstFreelistHolder = newHolder;

		InsertRB(newHolder->FirstNode->RBElement());

		// Callback is defined in memory manager.
		m_CallbackFn();

		return static_cast<void*>(newHolder->FirstNode);
	}

	void* FreelistRedBlackAllocator::FindFreeNodeAddress(U64 sizeBytes)
	{
		// Find fitting node using Red Black Tree (this is best fit).
		constexpr static U64 nodeHeaderSize = FreelistNode::HeaderSize();

		RedBlackTreeElement* treeNode = m_TreeHead;
		RedBlackTreeElement* potentialNode = m_NullTreeElement;
		while (treeNode != m_NullTreeElement)
		{
			if (treeNode->SizeBytes() > sizeBytes)
			{
				potentialNode = treeNode;
				treeNode = treeNode->Left;
			}
			else if (treeNode->SizeBytes() < sizeBytes)
			{
				treeNode = treeNode->Right;
			}
			else return reinterpret_cast<U8*>(treeNode) - nodeHeaderSize;
		}
		if (potentialNode != m_NullTreeElement) return reinterpret_cast<U8*>(potentialNode) - nodeHeaderSize;

		return nullptr;
	}

	void FreelistRedBlackAllocator::SplitFreelist(FreelistNode* node, U64 sizeBytes)
	{
		constexpr static U64 payloadOffset = FreelistNode::PayloadOffset();
		constexpr static U64 minNodeSize = FreelistNode::MinSize();

		U8* newNodeMemoryAddress = reinterpret_cast<U8*>(node) + payloadOffset + sizeBytes;
		U8* alignedNewNodeMemoryAddress = MemoryUtils::AlignPointer(newNodeMemoryAddress, alignof(void*));
		U8 offset = static_cast<U8>((alignedNewNodeMemoryAddress - newNodeMemoryAddress) & 0xFF);

		U64 newChunkSize = node->RBElement()->SizeBytes() - (sizeBytes + offset);
		if (newChunkSize <= minNodeSize) return;
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(alignedNewNodeMemoryAddress, newChunkSize));
		U64 nodeChangedSize = node->RBElement()->SizeBytes() - newChunkSize;
		node->RBElement()->SetSizeBytes(nodeChangedSize);
		newNode->Next = node->Next;
		if (newNode->Next) newNode->Next->Prev = newNode;
		node->Next = newNode;
		newNode->Prev = node;
		InsertRB(newNode->RBElement());
	}

	void FreelistRedBlackAllocator::MergeFreelist(FreelistNode* node)
	{
		// When this function executes, node is not free.
		constexpr static U64 payloadOffset = FreelistNode::PayloadOffset();
		ClearRB(node->RBElement());
		// Might seem strange, but node->Next might exist in different region of memory, because of expansion.
		if (node->Next && node->Next->IsFree() && node->IsNeighbourOf(node->Next))
		{
			U64 nodeChangedSize = node->RBElement()->SizeBytes() + node->Next->RBElement()->SizeBytes() + payloadOffset;
			DeleteRB(node->Next->RBElement());

			node->RBElement()->SetSizeBytes(nodeChangedSize);
			if (node->Next->Next) node->Next->Next->Prev = node;
			node->Next = node->Next->Next;
		}
		if (node->Prev && node->Prev->IsFree() && node->Prev->IsNeighbourOf(node))
		{
			U64 nodeChangedSize = node->Prev->RBElement()->SizeBytes() + node->RBElement()->SizeBytes() + payloadOffset;
			DeleteRB(node->Prev->RBElement());

			node->Prev->RBElement()->SetSizeBytes(nodeChangedSize);
			if (node->Next) node->Next->Prev = node->Prev;
			node->Prev->Next = node->Next;
			ClearRB(node->Prev->RBElement());
			InsertRB(node->Prev->RBElement());
		}
		else
		{
			InsertRB(node->RBElement());
		}
	}

	void FreelistRedBlackAllocator::InsertRB(RedBlackTreeElement* element)
	{
		// Search to find correct position of element.
		RedBlackTreeElement* searchElement = m_TreeHead;
		RedBlackTreeElement* potentialParent = m_NullTreeElement;
		while (searchElement != m_NullTreeElement)
		{
			potentialParent = searchElement;
			if (searchElement->SizeBytes() > element->SizeBytes())
				searchElement = searchElement->Left;
			else
				searchElement = searchElement->Right;
		}
		element->Parent = potentialParent;
		if (potentialParent == m_NullTreeElement)
		{
			m_TreeHead = element;
		}
		else if (element->SizeBytes() < potentialParent->SizeBytes())
		{
			potentialParent->Left = element;
		}
		else
		{
			potentialParent->Right = element;
		}
		element->Left = element->Right = m_NullTreeElement;

		FixTreeInsert(element);
	}

	void FreelistRedBlackAllocator::DeleteRB(RedBlackTreeElement* element)
	{
		RedBlackTreeElement* y, * x;
		y = element;
		x = m_NullTreeElement;

		bool yOriginalColor = y->IsRed();
		if (element->Left == m_NullTreeElement)
		{
			x = element->Right;
			TransplantRB(element, element->Right);
		} 
		else if (element->Right == m_NullTreeElement)
		{
			x = element->Left;
			TransplantRB(element, element->Left);
		}
		else
		{
			y = reinterpret_cast<RedBlackTreeElement*>(MinimumRB(element->Right));
			yOriginalColor = y->IsRed();
			x = y->Right;
			if (y->Parent == element)
			{
				x->Parent = y;
			}
			else
			{
				TransplantRB(y, y->Right);
				y->Right = element->Right;
				y->Right->Parent = y;
			}
			TransplantRB(element, y);
			y->Left = element->Left;
			y->Left->Parent = y;
			if (y->IsRed() != element->IsRed()) y->ChangeColor();
		}
		// if yOriginalColor is black.
		if (yOriginalColor == false)
			FixTreeDelete(x);
	}


	void FreelistRedBlackAllocator::FixTreeInsert(RedBlackTreeElement* element)
	{
		while (element->Parent->IsRed())
		{
			if (element->Parent == element->Parent->Parent->Left)
			{
				RedBlackTreeElement* uncle = element->Parent->Parent->Right;
				if (uncle->IsRed())
				{
					element->Parent->SetBlackColor();
					uncle->SetBlackColor();
					element->Parent->Parent->SetRedColor();
					element = element->Parent->Parent;
				}
				else
				{
					if (element == element->Parent->Right)
					{
						element = element->Parent;
						LeftRotation(element);
					}
					element->Parent->SetBlackColor();
					element->Parent->Parent->SetRedColor();
					RightRotation(element->Parent->Parent);
				}
			}
			else
			{
				RedBlackTreeElement* uncle = element->Parent->Parent->Left;
				if (uncle->IsRed())
				{
					element->Parent->SetBlackColor();
					uncle->SetBlackColor();
					element->Parent->Parent->SetRedColor();
					element = element->Parent->Parent;
				}
				else
				{
					if (element == element->Parent->Left)
					{
						element = element->Parent;
						RightRotation(element);
					}
					element->Parent->SetBlackColor();
					element->Parent->Parent->SetRedColor();
					LeftRotation(element->Parent->Parent);
				}
			}
		}
		m_TreeHead->SetBlackColor();
	}

	void FreelistRedBlackAllocator::FixTreeDelete(RedBlackTreeElement* element)
	{
		while (element != m_TreeHead && !element->IsRed())
		{
			if (element == element->Parent->Left)
			{
				RedBlackTreeElement* sibling = element->Parent->Right;
				if (sibling->IsRed())
				{
					sibling->SetBlackColor();
					element->Parent->SetRedColor();
					LeftRotation(element->Parent);
					sibling = element->Parent->Right;
				}
				if (!sibling->Left->IsRed() && !sibling->Right->IsRed())
				{
					sibling->SetRedColor();
					element = element->Parent;
				}
				else
				{
					if (!sibling->Right->IsRed())
					{
						sibling->Left->SetBlackColor();
						sibling->SetRedColor();
						RightRotation(sibling);
						sibling = element->Parent->Right;
					}
					(element->Parent->IsRed()) ? sibling->SetRedColor() : sibling->SetBlackColor();
					element->Parent->SetBlackColor();
					sibling->Right->SetBlackColor();
					LeftRotation(element->Parent);
					element = m_TreeHead;
				}
			}
			else
			{
				RedBlackTreeElement* sibling = element->Parent->Left;
				if (sibling->IsRed())
				{
					sibling->SetBlackColor();
					element->Parent->SetRedColor();
					RightRotation(element->Parent);
					sibling = element->Parent->Left;
				}
				if (!sibling->Left->IsRed() && !sibling->Right->IsRed())
				{
					sibling->SetRedColor();
					element = element->Parent;
				}
				else
				{
					if (!sibling->Left->IsRed())
					{
						sibling->Right->SetBlackColor();
						sibling->SetRedColor();
						LeftRotation(sibling);
						sibling = element->Parent->Left;
					}
					(element->Parent->IsRed()) ? sibling->SetRedColor() : sibling->SetBlackColor();
					element->Parent->SetBlackColor();
					sibling->Left->SetBlackColor();
					RightRotation(element->Parent);
					element = m_TreeHead;
				}
			}
		}
		element->SetBlackColor();
	}

	void FreelistRedBlackAllocator::TransplantRB(RedBlackTreeElement* U, RedBlackTreeElement* V)
	{
		if (U->Parent == m_NullTreeElement) m_TreeHead = V;
		else if (U == U->Parent->Left) U->Parent->Left = V;
		else U->Parent->Right = V;
		V->Parent = U->Parent;
	}

	void* FreelistRedBlackAllocator::MinimumRB(RedBlackTreeElement* element)
	{
		while (element->Left != m_NullTreeElement)
			element = element->Left;
		return element;
	}

	void FreelistRedBlackAllocator::LeftRotation(RedBlackTreeElement* element)
	{
		RedBlackTreeElement* sibling = element->Right;
		element->Right = sibling->Left;

		// Turn sibling's left subtree into element's right subtree.
		if (sibling->Left != m_NullTreeElement) sibling->Left->Parent = element;
		sibling->Parent = element->Parent;
		
		if (element->Parent == m_NullTreeElement) m_TreeHead = sibling;
		else if (element == element->Parent->Left) element->Parent->Left = sibling;
		else element->Parent->Right = sibling;

		sibling->Left = element;
		element->Parent = sibling;
	}

	void FreelistRedBlackAllocator::RightRotation(RedBlackTreeElement* element)
	{
		RedBlackTreeElement* sibling = element->Left;
		element->Left = sibling->Right;
		
		// Turn sibling's right subtree into element's left subtree.
		if (sibling->Right != m_NullTreeElement) sibling->Right->Parent = element;
		sibling->Parent = element->Parent;

		if (element->Parent == m_NullTreeElement) m_TreeHead = sibling;
		else if (element == element->Parent->Right) element->Parent->Right = sibling;
		else element->Parent->Left = sibling;
		
		sibling->Right = element;
		element->Parent = sibling;
	}

	void FreelistRedBlackAllocator::ClearRB(RedBlackTreeElement* element)
	{
		element->Left = element->Right = element->Parent = nullptr;
		element->SetRedColor();
	}


	FreelistRedBlackAllocator::~FreelistRedBlackAllocator()
	{
		FreelistHolder* toBeDeleted = m_FirstFreelistHolder;
		while (toBeDeleted->Next)
		{
			void* memory = reinterpret_cast<void*>(toBeDeleted);
			toBeDeleted = toBeDeleted->Next;
			MemoryUtils::FreeAligned(memory);
		}
		MemoryUtils::FreeAligned(reinterpret_cast<void*>(m_NullTreeElement));
	}
}