#include "enginepch.h"

#include "BVHTree.h"

namespace Engine::WIP::Physics::Newest
{
	BVHTree2D::BVHTree2D()
	{
		Clear();
	}

	void BVHTree2D::Clear()
	{
		m_Nodes.clear();
		m_TreeRoot = BVHNode::NULL_NODE;
		// Allocate initial buffer for nodes.
		static U32 initialNodesAmount = 16;
		Resize(0, initialNodesAmount);
		m_FreeList = 0;
		m_FreeNodesCount = U32(m_Nodes.size());
	}
	
    U32 BVHTree2D::Insert(void* payload, const AABB2D& bounds)
	{
		// Allocate a new leaf.
		U32 leafIndex = AllocateNode();
		m_Nodes[leafIndex].Payload = payload;
		m_Nodes[leafIndex].NodeBounds = bounds;
		// Enlarge aabb so fewer relocation operations is required (box2d-like).
		glm::vec2 AABB2DExpansion{ s_AABB2DGrowth };
		m_Nodes[leafIndex].NodeBounds.Expand(AABB2DExpansion);

		InsertLeaf(leafIndex);

		return leafIndex;
	}

	void BVHTree2D::Remove(U32 nodeId)
	{
		RemoveLeaf(nodeId);
		FreeNode(nodeId);
	}

	void BVHTree2D::InsertLeaf(U32 leafId)
	{
		// If this is the first insertion (tree is empty).
		if (m_TreeRoot == BVHNode::NULL_NODE)
		{
			m_TreeRoot = leafId;
			m_Nodes[m_TreeRoot].Parent = BVHNode::NULL_NODE;
			return;
		}
		// Else we need to find the best place (neighbour)
		// for the given leafId, based on some cost function.

		// Find the best neighbour.
		//I32 bestNeighbour = FindBestNeighbour(leafId);
		U32 bestNeighbour = FindBestNeighbourBnB(leafId);

		// Construct new Parent.
		U32 oldParent = m_Nodes[bestNeighbour].Parent;
		U32 newParent = AllocateNode();
		m_Nodes[newParent].Parent = oldParent;
		m_Nodes[newParent].Height = m_Nodes[bestNeighbour].Height + 1;
		const AABB2D& leafAABB2D = m_Nodes[leafId].NodeBounds;
		m_Nodes[newParent].NodeBounds = AABB2D{ leafAABB2D, m_Nodes[bestNeighbour].NodeBounds };
		m_Nodes[newParent].Payload = nullptr;

		m_Nodes[newParent].LeftChild = bestNeighbour;
		m_Nodes[newParent].RightChild = leafId;
		m_Nodes[leafId].Parent = newParent;
		m_Nodes[bestNeighbour].Parent = newParent;

		if (oldParent != BVHNode::NULL_NODE)
		{
			if (bestNeighbour == m_Nodes[oldParent].LeftChild)
			{
				m_Nodes[oldParent].LeftChild = newParent;
			}
			else
			{
				m_Nodes[oldParent].RightChild = newParent;
			}
		}
		// If the sibling was the root.
		else
		{
			m_TreeRoot = newParent;
		}

		// Reshape the tree.
		ReshapeTree(m_Nodes[leafId].Parent);
	}
	
	void BVHTree2D::RemoveLeaf(U32 leafId)
	{
		// If leaf is root, the become empty.
		if (leafId == m_TreeRoot)
		{
			m_TreeRoot = BVHNode::NULL_NODE;
			return;
		}

		// Else neighbour's parent becomes neighbour itself,
		// the parent shall no longer exists, since it always 
		// was just an internal node.
		U32 parent = m_Nodes[leafId].Parent;
		U32 neighbour;
		if (m_Nodes[parent].LeftChild == leafId)
		{
			neighbour = m_Nodes[parent].RightChild;
		}
		else
		{
			neighbour = m_Nodes[parent].LeftChild;
		}

		U32 grandparent = m_Nodes[parent].Parent;
		if (grandparent != BVHNode::NULL_NODE)
		{
			if (m_Nodes[grandparent].LeftChild == parent)
			{
				m_Nodes[grandparent].LeftChild = neighbour;
			}
			else
			{
				m_Nodes[grandparent].RightChild = neighbour;
			}
			m_Nodes[neighbour].Parent = grandparent;
			FreeNode(parent);
			// Reshape the tree.
			ReshapeTree(grandparent);
		}
		// Else parent was the root, so root becomes neighbour.
		else
		{
			m_TreeRoot = neighbour;
			m_Nodes[neighbour].Parent = BVHNode::NULL_NODE;
			FreeNode(parent);
		}
	}
	
	bool BVHTree2D::Move(U32 nodeId, const AABB2D& bounds, const glm::vec2& velocity)
	{
		// The bounds currently stored in the tree
		// might be larger than expanded version of object's bounds
		// (due to this function), so we expand the provided bounds again.
		glm::vec2 AABB2DExpansion{ s_AABB2DGrowth };
		AABB2D expandedAABB2D = bounds;
		expandedAABB2D.Expand(AABB2DExpansion);
		// Account for the movement of the object 
		// this is why stored bounds might be bigger).
		glm::vec2 dynamicAABB2DExpansion{ s_DynamicAABB2DGrowth * velocity };
		expandedAABB2D.ExpandSigned(dynamicAABB2DExpansion);

		const AABB2D& treeAABB2D = m_Nodes[nodeId].NodeBounds;
		if (treeAABB2D.Contains(bounds))
		{
			// Now if tree bounds are not too large,
			// we do not need to reinsert it
			// (it may be large if object moved too fast before).
			AABB2D hugeAABB2D = expandedAABB2D;
			glm::vec2 hugeExpansion{ s_AABB2DGrowth * 4.0f };
			hugeAABB2D.Expand(hugeExpansion);
			if (hugeAABB2D.Contains(treeAABB2D))
			{
				// tree bounds are not too large.
				return false;
			}
		}

		RemoveLeaf(nodeId);
		m_Nodes[nodeId].NodeBounds = expandedAABB2D;
		InsertLeaf(nodeId);
		m_Nodes[nodeId].Moved = true;

		return true;
	}

	U32 BVHTree2D::FindBestNeighbourBnB(U32 leafId)
	{
		const AABB2D& leafAABB2D = m_Nodes[leafId].NodeBounds;
		F32 leafAABB2DArea = leafAABB2D.GetPerimeter();
        // Find the best neighbour.
		std::queue<U32> toProcess;
		std::queue<F32> inheritedCosts;
		toProcess.push(m_TreeRoot);
		inheritedCosts.push(0.0f);
		U32 bestNeighbour = m_TreeRoot;
		F32 bestCost = std::numeric_limits<F32>::max();
		while (!toProcess.empty())
		{
			U32 curr = toProcess.front(); toProcess.pop();
			F32 inheritedCost = inheritedCosts.front(); inheritedCosts.pop();
			if (curr == BVHNode::NULL_NODE) continue;

			F32 currCost = AABB2D{ leafAABB2D, m_Nodes[curr].NodeBounds }.GetPerimeter();
			F32 currCostInherited = currCost + inheritedCost;
			if (currCostInherited < bestCost)
			{
				bestCost = currCostInherited;
				bestNeighbour = curr;
				// Check if we also need to process children of `curr`.
				F32 newInheritedCost = inheritedCost + currCost - m_Nodes[curr].NodeBounds.GetPerimeter();
				F32 lowerBound = leafAABB2DArea + newInheritedCost;
				if (lowerBound < bestCost)
				{
					toProcess.push(m_Nodes[bestNeighbour].LeftChild);
					toProcess.push(m_Nodes[bestNeighbour].RightChild);
					inheritedCosts.push(newInheritedCost);
					inheritedCosts.push(newInheritedCost);
				}
			}
		}
		ENGINE_CORE_ASSERT(inheritedCosts.empty(), "Unsynchronized.")
		return bestNeighbour;
	}

	void BVHTree2D::ReshapeTree(U32 nodeId)
	{
		U32 currentId = nodeId;
		while (currentId != BVHNode::NULL_NODE)
		{
			U32 leftChild = m_Nodes[currentId].LeftChild;
			U32 rightChild = m_Nodes[currentId].RightChild;
			m_Nodes[currentId].Height = 1 + Math::Max(m_Nodes[leftChild].Height, m_Nodes[rightChild].Height);
			m_Nodes[currentId].NodeBounds = AABB2D{ m_Nodes[leftChild].NodeBounds, m_Nodes[rightChild].NodeBounds };
			
			currentId = RebalanceBnB(currentId);

			currentId = m_Nodes[currentId].Parent;
		}
	}

	void BVHTree2D::Resize(U32 startIndex, U32 endIndex)
	{
		m_Nodes.resize(m_Nodes.size() + (endIndex - startIndex));
		// Create a linked list of free nodes.
		for (U32 i = startIndex; i < endIndex - 1; i++)
		{
			m_Nodes[i].Next = I32(i + 1);
			m_Nodes[i].Height = -1;
		}
		m_Nodes.back().Next = BVHNode::NULL_NODE;
		m_Nodes.back().Height = -1;
	}

	U32 BVHTree2D::AllocateNode()
	{
		// If no more free nodes.
		if (m_FreeList == BVHNode::NULL_NODE)
		{
			U32 currentCapacity = U32(m_Nodes.size());
			Resize(currentCapacity, 2 * currentCapacity);
			m_FreeList = I32(currentCapacity);
			m_FreeNodesCount = I32(currentCapacity);
		}

		U32 freeLeaf = m_FreeList;
		m_FreeList = m_Nodes[m_FreeList].Next;
		m_Nodes[freeLeaf].Parent = BVHNode::NULL_NODE;
		m_Nodes[freeLeaf].Height = 0;
		m_FreeNodesCount--;
		return freeLeaf;
	}
	
	void BVHTree2D::FreeNode(U32 nodeId)
	{
		m_Nodes[nodeId].Next = m_FreeList;
		m_Nodes[nodeId].Height = -1;
		m_FreeList = nodeId;
		m_FreeNodesCount++;
	}

	BVHTree2D::TreeRotation BVHTree2D::FindRotation(U32 siblingToChange, U32 siblingAsChange)
	{
		F32 toChangeSA = m_Nodes[siblingToChange].NodeBounds.GetPerimeter();
		// LSA - if we changed left node to `asChange`, RSA otherwise.
		F32 changedLSA = std::numeric_limits<F32>::max(); F32 changedRSA = std::numeric_limits<F32>::max();
		BVHNode toChange = m_Nodes[siblingToChange];
		BVHNode asChange = m_Nodes[siblingAsChange];
		if (toChange.LeftChild != BVHNode::NULL_NODE) changedRSA = AABB2D{m_Nodes[toChange.LeftChild].NodeBounds, asChange.NodeBounds}.GetPerimeter();
		if (toChange.RightChild != BVHNode::NULL_NODE) changedLSA = AABB2D{m_Nodes[toChange.RightChild].NodeBounds, asChange.NodeBounds}.GetPerimeter();

		if (changedLSA < toChangeSA && changedLSA < changedRSA) return TreeRotation{siblingAsChange, toChange.LeftChild, changedLSA};
		if (changedRSA < toChangeSA && changedRSA < changedLSA) return TreeRotation{siblingAsChange, toChange.RightChild, changedRSA};
		return {};
	}
	
	void BVHTree2D::Rotate(U32 higherI, U32 lowerI)
	{
		BVHNode h = m_Nodes[higherI];
		BVHNode l = m_Nodes[lowerI];

		ENGINE_CORE_ASSERT(h.Parent == m_Nodes[l.Parent].Parent, "Invalid parameters.")
		
		BVHNode hP = m_Nodes[h.Parent];
		BVHNode lP = m_Nodes[l.Parent];
		if (hP.LeftChild == higherI) hP.LeftChild = lowerI;
		else hP.RightChild = lowerI;
		if (lP.LeftChild == lowerI) lP.LeftChild = higherI;
		else lP.RightChild = higherI;
		std::swap(h.Parent, l.Parent);
		lP.NodeBounds = AABB2D{m_Nodes[lP.LeftChild].NodeBounds, m_Nodes[lP.RightChild].NodeBounds};
	}

	U32 BVHTree2D::RebalanceBnB(U32 nodeId)
	{
		BVHNode a = m_Nodes[nodeId];
		if (a.Height < 2) return nodeId;
		
		auto&& [hA, lB, SAA] = FindRotation(a.RightChild, a.LeftChild);
		auto&& [hB, lA, SAB] = FindRotation(a.LeftChild, a.RightChild);

		if (SAA < SAB) Rotate(hA, lB);
		else if (SAB < SAA) Rotate(hB, lA);

		return nodeId;
	}

	F32 BVHTree2D::ComputeTotalCost() const
	{
		F32 cost = 0.0f;
		for (auto& node : m_Nodes)
		{
			if (node.Height > 0) cost += node.NodeBounds.GetPerimeter();
		}
		return cost;
	}
}