#include "enginepch.h"

#include "FreelistRedBlackTreeAllocator.h"
#include "MemoryUtils.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	FreelistRedBlackAllocator::FreelistRedBlackAllocator(U32 sizeBytes)
	{
		ENGINE_ASSERT(sizeBytes >= FreelistNode::MinSize(), "Freelist allocator must be larger.");
		void* freelistMemory = MemoryUtils::AllocAligned(sizeBytes);
		m_FreelistMemory = reinterpret_cast<U8*>(freelistMemory);
		m_FirstNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistMemory, sizeBytes));
		m_TreeHead = m_FirstNode->RBElement();
		m_LastNode = m_FirstNode;
	}

	void* FreelistRedBlackAllocator::AllocAligned(U32 sizeBytes, U16 alignment)
	{
		constexpr static U32 payloadOffset = FreelistNode::PayloadOffset();
		constexpr static U32 treeElementHeader = RedBlackTreeElement::HeaderSize();

		// Since at some point we want to "return" node, 
		// we have to make sure, that essencial tree data will fit.
		U32 actualBytes = std::max(sizeBytes, treeElementHeader) + alignment;
		// Find free node with sufficient size. 
		FreelistNode* node = reinterpret_cast<FreelistNode*>(FindFreeNodeAddress(actualBytes));

		// Allocate extra space if no freeNode was found.
		if (node == nullptr)
			node = reinterpret_cast<FreelistNode*>(ExpandFreelist(actualBytes));

		// Subdivide memory to smaller chunks and mark used node.
		SplitFreelist(node, actualBytes);
		// If we have split the last node, it should change.
		if (m_LastNode->Next) m_LastNode = m_LastNode->Next;
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
		constexpr static U32 payloadOffset = FreelistNode::PayloadOffset();

		// Get actual address
		U8* alignedMemory = reinterpret_cast<U8*>(memory);
		ptrdiff_t offset = alignedMemory[-1];
		U8* rawMemory = alignedMemory - offset;

		FreelistNode* node = reinterpret_cast<FreelistNode*>(rawMemory - payloadOffset);
		node->SetFree();

		// Try merge free neighbour nodes.
		MergeFreelist(node);
	}
	
	void* FreelistRedBlackAllocator::GetInitializedNode(void* memory, U32 sizeBytes)
	{
		FreelistNode* node = reinterpret_cast<FreelistNode*>(memory);
		node->Next = node->Prev = nullptr;
		InitializeRBElement(node, sizeBytes);
		node->SetFree(); 
		return node;
	}

	void FreelistRedBlackAllocator::InitializeRBElement(FreelistNode* node, U32 sizeBytes)
	{
		constexpr static U32 payloadOffset = FreelistNode::PayloadOffset();
		RedBlackTreeElement* rbElement = reinterpret_cast<RedBlackTreeElement*>(node->RBElement());
		rbElement->SizeAndFlags = sizeBytes - payloadOffset;
		rbElement->Parent = rbElement->Left = rbElement->Right = nullptr;
		rbElement->SetRedColor();
	}

	void* FreelistRedBlackAllocator::ExpandFreelist(U32 sizeBytes)
	{
		// Allocate additional memory (according to config).
		// This allocation can return address which is less than m_FirstNode.
		sizeBytes = std::max(sizeBytes, RBFREELIST_ALLOCATOR_INCREMENT_BYTES);
		void* freelistExtension = MemoryUtils::AllocAligned(sizeBytes);
		m_AdditionalAllocations.push_back(freelistExtension);
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(freelistExtension, sizeBytes));
		if (newNode > m_LastNode)
		{
			m_LastNode->Next = newNode;
			newNode->Prev = m_LastNode;
			m_LastNode = newNode;
		}
		else if (newNode < m_FirstNode)
		{
			newNode->Next = m_FirstNode;
			m_FirstNode->Prev = newNode;
			m_FirstNode = newNode;
		}
		else
		{
			// It is very safe to assume that this will never happen.
			ENGINE_CORE_FATAL("Impossible allocation in FreelistAllocator.");
		}

		InsertRB(newNode->RBElement());

		return freelistExtension;
	}

	void* FreelistRedBlackAllocator::FindFreeNodeAddress(U32 sizeBytes)
	{
		// Find fitting node using Red Black Tree (this is best fit).
		constexpr static U32 nodeHeaderSize = FreelistNode::HeaderSize();

		RedBlackTreeElement* treeNode = m_TreeHead;
		RedBlackTreeElement* potentialNode = nullptr;
		if (!treeNode) return nullptr;
		while (treeNode)
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
		if (potentialNode != nullptr) return reinterpret_cast<U8*>(potentialNode) - nodeHeaderSize;
		// It shall never happen.
		ENGINE_ERROR("Red Black Tree is broken.");
		return nullptr;
	}

	void FreelistRedBlackAllocator::SplitFreelist(FreelistNode* node, U32 sizeBytes)
	{
		constexpr static U32 payloadOffset = FreelistNode::PayloadOffset();
		constexpr static U32 minNodeSize = FreelistNode::MinSize();

		U8* newNodeMemoryAddress = reinterpret_cast<U8*>(node) + payloadOffset + sizeBytes;
		U8* alignedNewNodeMemoryAddress = MemoryUtils::AlignPointer(newNodeMemoryAddress, alignof(void*));
		U8 offset = static_cast<U8>((alignedNewNodeMemoryAddress - newNodeMemoryAddress) & 0xFF);

		U32 newChunkSize = node->RBElement()->SizeBytes() - (sizeBytes + offset);
		if (newChunkSize <= minNodeSize) return;
		FreelistNode* newNode = reinterpret_cast<FreelistNode*>(GetInitializedNode(alignedNewNodeMemoryAddress, newChunkSize));
		U32 nodeChangedSize = node->RBElement()->SizeBytes() - newChunkSize;
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
		constexpr static U32 payloadOffset = FreelistNode::PayloadOffset();
		node->RBElement()->Clear();
		// Might seem strange, but node->Next might exist in different region of memory, because of expansion.
		if (node->Next && node->Next->IsFree() && node->IsNeighbourOf(node->Next))
		{
			U32 nodeChangedSize = node->RBElement()->SizeBytes() + node->Next->RBElement()->SizeBytes() + payloadOffset;
			node->RBElement()->SetSizeBytes(nodeChangedSize);

			DeleteRB(node->Next->RBElement());
			if (node->Next->Next) node->Next->Next->Prev = node;
			node->Next = node->Next->Next;
			InsertRB(node->RBElement());
		}
		else
		{
			InsertRB(node->RBElement());
		}
		if (node->Prev && node->Prev->IsFree() && node->Prev->IsNeighbourOf(node))
		{
			U32 nodeChangedSize = node->Prev->RBElement()->SizeBytes() + node->RBElement()->SizeBytes() + payloadOffset;
			node->Prev->RBElement()->SetSizeBytes(nodeChangedSize);

			DeleteRB(node->RBElement());
			if (node->Next) node->Next->Prev = node->Prev;
			node->Prev->Next = node->Next;
			DeleteRB(node->Prev->RBElement());
			InsertRB(node->Prev->RBElement());
		}
	}

	void FreelistRedBlackAllocator::InsertRB(RedBlackTreeElement* element)
	{
		// If nothing in the tree (probably impossible state).
		if (m_TreeHead == nullptr)
		{
			element->SetBlackColor();
			m_TreeHead = element;
			return;
		}

		// Search to find correct position of element.
		RedBlackTreeElement* searchElement = m_TreeHead;
		RedBlackTreeElement* potentialParent = m_TreeHead;
		while (searchElement != nullptr)
		{
			potentialParent = searchElement;
			if (searchElement->SizeBytes() > element->SizeBytes())
				searchElement = searchElement->Left;
			else
				searchElement = searchElement->Right;
		}
		element->Parent = potentialParent;
		if (element->SizeBytes() < potentialParent->SizeBytes())
		{
			element->Parent->Left = element;
		}
		else
		{
			element->Parent->Right = element;
		}
		if (element->Parent->Parent == nullptr) return;
		FixTreeInsert(element);
	}

	void FreelistRedBlackAllocator::DeleteRB(RedBlackTreeElement* element)
	{
		RedBlackTreeElement* y, * x;
		y = x = nullptr;

		y = element;
		bool yOriginalColor = y->IsRed();
		if (element->Left == nullptr)
		{
			x = element->Right;
			TransplantRB(element, element->Right);
		} 
		else if (element->Right == nullptr)
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
			if (!(y->IsRed() ^ element->IsRed())) y->ChangeColor();
		}
		// yOriginalColor is black.
		if (yOriginalColor == false)
			FixTreeDelete(x);
	}

	void FreelistRedBlackAllocator::TransplantRB(RedBlackTreeElement* U, RedBlackTreeElement* V)
	{
		if (U->Parent == nullptr) m_TreeHead = V;
		else if (U == U->Parent->Left) U->Parent->Left = V;
		else U->Parent->Right = V;
		if (V) V->Parent = U->Parent;
	}

	void* FreelistRedBlackAllocator::MinimumRB(RedBlackTreeElement* element)
	{
		while (element->Left)
			element = element->Left;
		return element;
	}

	void FreelistRedBlackAllocator::FixTreeInsert(RedBlackTreeElement* element)
	{
		while (element->Parent->IsRed() && element != m_TreeHead)
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
					if (!(sibling->IsRed() ^ element->Parent->IsRed())) sibling->ChangeColor();
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
					if (!(sibling->IsRed() ^ element->Parent->IsRed())) sibling->ChangeColor();
					element->Parent->SetBlackColor();
					sibling->Left->SetBlackColor();
					RightRotation(element->Parent);
					element = m_TreeHead;
				}
			}
		}
		element->SetBlackColor();
	}

	void FreelistRedBlackAllocator::LeftRotation(RedBlackTreeElement* element)
	{
		RedBlackTreeElement* sibling = element->Right;
		element->Right = sibling->Left;

		// Turn sibling's left subtree into element's right subtree.
		if (sibling->Left != nullptr) sibling->Left->Parent = element;
		sibling->Parent = element->Parent;
		
		if (element->Parent == nullptr)	m_TreeHead = sibling; 
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
		if (sibling->Right != nullptr) sibling->Right->Parent = element;
		sibling->Parent = element->Parent;

		if (element->Parent == nullptr) m_TreeHead = sibling;
		else if (element == element->Parent->Right) element->Parent->Right = sibling;
		else element->Parent->Left = sibling;
		
		sibling->Right = element;
		element->Parent = sibling;
	}

	FreelistRedBlackAllocator::~FreelistRedBlackAllocator()
	{
		MemoryUtils::FreeAligned(static_cast<void*>(m_FreelistMemory));
		for (auto& al : m_AdditionalAllocations)
			MemoryUtils::FreeAligned(al);
	}
}