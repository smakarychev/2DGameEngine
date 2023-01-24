#pragma once

#include "Collider.h"

#include <stack>
#include <queue>


namespace Engine
{
	// Defined in Rendering/Drawers/BVHTreeDrawer.h
	class BVHTreeDrawer;
}

namespace Engine::WIP::Physics
{
	template <typename Bounds>
	struct BVHNode
	{
		static constexpr auto NULL_NODE = -1;
		static constexpr auto NULL_ITEM = -1;
		// Stores enlarged bounds (box2d like).
		Bounds NodeBounds;
		I32 LeftChild = NULL_NODE;
		I32 RightChild = NULL_NODE;
		// 0 for leaves, -1 if node is free.
		I32 Height = -1;
		// If node is free, stores the index of next free node, 
		// stores parent otherwise.
		union
		{
			I32 Parent;
			I32 Next = NULL_NODE;
		};
		// It is assumed that Payload is of type Collider2D.
		void* Payload = nullptr;
		bool Moved = false;

		bool IsLeaf() const
		{
			return Height == 0;
		}
	};

	template <typename Bounds>
	class BVHTree2D
	{
		friend class Engine::BVHTreeDrawer;
	public:
		struct TreeRotation
		{
			I32 High = BVHNode<Bounds>::NULL_NODE;
			I32 Low = BVHNode<Bounds>::NULL_NODE;
			F32 SA = std::numeric_limits<F32>::max();
		};
	public:
		BVHTree2D();
		void Clear();
		// `itemId` is id of object in some external container,
		// Returns the index of node.
		I32 Insert(void* payload, const Bounds& bounds);

		// `nodeId` is returned by Insert().
		void Remove(I32 nodeId);

		// `nodeId` is returned by Insert(), `velocity` here is a 
		// mere measure of displacement, not strictly related 
		// to physical velocity of the object.
		bool Move(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity);

		template <typename Callback>
		void Query(const Callback& callback, const Bounds& bounds);

		bool IsMoved(I32 nodeId) const { return m_Nodes[nodeId].Moved; }
		void ResetMoved(I32 nodeId) { m_Nodes[nodeId].Moved = false; }
		void SetMoved(I32 nodeId) { m_Nodes[nodeId].Moved = true; }
		void* GetPayload(I32 nodeId) const { return m_Nodes[nodeId].Payload; }
		const Bounds& GetBounds(I32 nodeId) const { return m_Nodes[nodeId].NodeBounds; }

		F32 ComputeTotalCost() const;
	private:
		void Resize(U32 startIndex, U32 endIndex);
		void InsertLeaf(I32 leafId);
		void RemoveLeaf(I32 leafId);
		I32 FindBestNeighbourBnB(I32 leafId);
		void ReshapeTree(I32 nodeId);
		I32 AllocateNode();
		void FreeNode(I32 nodeId);
		TreeRotation FindRotation(I32 siblingToChange, I32 siblingAsChange);
		void Rotate(I32 higherI, I32 lowerI);
		I32 RebalanceBnB(I32 nodeId);
	private:
		// Used to enlarge bounds so fewer reinsertions performed.
		static constexpr auto s_BoundsGrowth = 0.1f;
		// Used to greatly enlarge bounds of rapidly moving objects.
		static constexpr auto s_DynamicBoundsGrowth = 4.0f;
		std::vector<BVHNode<Bounds>> m_Nodes;
		I32 m_FreeList = BVHNode<Bounds>::NULL_NODE;
		I32 m_FreeNodesCount = 0;
		I32 m_TreeRoot = BVHNode<Bounds>::NULL_NODE;
	};

	template<typename Bounds>
	BVHTree2D<Bounds>::BVHTree2D()
	{
		Clear();
	}

	template<typename Bounds>
	inline void BVHTree2D<Bounds>::Clear()
	{
		m_Nodes.clear();
		m_TreeRoot = BVHNode<Bounds>::NULL_NODE;
		// Allocate initial buffer for nodes.
		static U32 initialNodesAmount = 16;
		Resize(0, initialNodesAmount);
		m_FreeList = 0;
		m_FreeNodesCount = I32(m_Nodes.size());
	}

	template<typename Bounds>
	template<typename Callback>
	void BVHTree2D<Bounds>::Query(const Callback& callback, const Bounds& bounds)
	{
		std::stack<I32> toProcess;
		toProcess.push(m_TreeRoot);

		while (toProcess.empty() == false)
		{
			I32 currentNode = toProcess.top(); toProcess.pop();
			if (currentNode == BVHNode<Bounds>::NULL_NODE)
			{
				continue;
			}
			// Test for intersection.
			if (m_Nodes[currentNode].NodeBounds.Intersects(bounds))
			{
				if (m_Nodes[currentNode].IsLeaf())
				{
					callback(currentNode);
				}
				// If it's not a leaf, process children.
				else
				{
					toProcess.push(m_Nodes[currentNode].LeftChild);
					toProcess.push(m_Nodes[currentNode].RightChild);
				}
			}

		}
	}

	template<typename Bounds>
	I32 BVHTree2D<Bounds>::Insert(void* payload, const Bounds& bounds)
	{
		// Allocate a new leaf.
		I32 leafIndex = AllocateNode();
		m_Nodes[leafIndex].Payload = payload;
		m_Nodes[leafIndex].NodeBounds = bounds;
		// Enlarge aabb so fewer relocation operations is required (box2d-like).
		glm::vec2 boundsExpansion{ s_BoundsGrowth };
		m_Nodes[leafIndex].NodeBounds.Expand(boundsExpansion);

		InsertLeaf(leafIndex);

		return leafIndex;
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::Remove(I32 nodeId)
	{
		RemoveLeaf(nodeId);
		FreeNode(nodeId);
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::InsertLeaf(I32 leafId)
	{
		// If this is the first insertion (tree is empty).
		if (m_TreeRoot == BVHNode<Bounds>::NULL_NODE)
		{
			m_TreeRoot = leafId;
			m_Nodes[m_TreeRoot].Parent = BVHNode<Bounds>::NULL_NODE;
			return;
		}
		// Else we need to find the best place (neighbour)
		// for the given leafId, based on some cost function.

		// Find the best neighbour.
		//I32 bestNeighbour = FindBestNeighbour(leafId);
		I32 bestNeighbour = FindBestNeighbourBnB(leafId);

		// Construct new Parent.
		I32 oldParent = m_Nodes[bestNeighbour].Parent;
		I32 newParent = AllocateNode();
		m_Nodes[newParent].Parent = oldParent;
		m_Nodes[newParent].Height = m_Nodes[bestNeighbour].Height + 1;
		const Bounds& leafBounds = m_Nodes[leafId].NodeBounds;
		m_Nodes[newParent].NodeBounds = Bounds{ leafBounds, m_Nodes[bestNeighbour].NodeBounds };
		m_Nodes[newParent].Payload = nullptr;

		m_Nodes[newParent].LeftChild = bestNeighbour;
		m_Nodes[newParent].RightChild = leafId;
		m_Nodes[leafId].Parent = newParent;
		m_Nodes[bestNeighbour].Parent = newParent;

		if (oldParent != BVHNode<Bounds>::NULL_NODE)
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

	template<typename Bounds>
	void BVHTree2D<Bounds>::RemoveLeaf(I32 leafId)
	{
		// If leaf is root, the become empty.
		if (leafId == m_TreeRoot)
		{
			m_TreeRoot = BVHNode<Bounds>::NULL_NODE;
			return;
		}

		// Else neighbour's parent becomes neighbour itself,
		// the parent shall no longer exists, since it always 
		// was just an internal node.
		I32 parent = m_Nodes[leafId].Parent;
		I32 neighbour;
		if (m_Nodes[parent].LeftChild == leafId)
		{
			neighbour = m_Nodes[parent].RightChild;
		}
		else
		{
			neighbour = m_Nodes[parent].LeftChild;
		}

		I32 grandparent = m_Nodes[parent].Parent;
		if (grandparent != BVHNode<Bounds>::NULL_NODE)
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
			m_Nodes[neighbour].Parent = BVHNode<Bounds>::NULL_NODE;
			FreeNode(parent);
		}
	}

	template<typename Bounds>
	bool BVHTree2D<Bounds>::Move(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity)
	{
		// The bounds currently stored in the tree
		// might be larger than expanded version of object's bounds
		// (due to this function), so we expand the provided bounds again.
		glm::vec2 boundsExpansion{ s_BoundsGrowth };
		Bounds expandedBounds = bounds;
		expandedBounds.Expand(boundsExpansion);
		// Account for the movement of the object 
		// this is why stored bounds might be bigger).
		glm::vec2 dynamicBoundsExpansion{ s_DynamicBoundsGrowth * velocity };
		expandedBounds.ExpandSigned(dynamicBoundsExpansion);

		const Bounds& treeBounds = m_Nodes[nodeId].NodeBounds;
		if (treeBounds.Contains(bounds))
		{
			// Now if tree bounds are not too large,
			// we do not need to reinsert it
			// (it may be large if object moved too fast before).
			Bounds hugeBounds = expandedBounds;
			glm::vec2 hugeExpansion{ s_BoundsGrowth * 4.0f };
			hugeBounds.Expand(hugeExpansion);
			if (hugeBounds.Contains(treeBounds))
			{
				// tree bounds are not too large.
				return false;
			}
		}

		RemoveLeaf(nodeId);
		m_Nodes[nodeId].NodeBounds = expandedBounds;
		InsertLeaf(nodeId);
		m_Nodes[nodeId].Moved = true;

		return true;
	}

	template <typename Bounds>
	I32 BVHTree2D<Bounds>::FindBestNeighbourBnB(I32 leafId)
	{
		const Bounds& leafBounds = m_Nodes[leafId].NodeBounds;
		F32 leafBoundsArea = leafBounds.GetPerimeter();
        // Find the best neighbour.
		std::queue<I32> toProcess;
		std::queue<F32> inheritedCosts;
		toProcess.push(m_TreeRoot);
		inheritedCosts.push(0.0f);
		I32 bestNeighbour = m_TreeRoot;
		F32 bestCost = std::numeric_limits<F32>::max();
		while (!toProcess.empty())
		{
			I32 curr = toProcess.front(); toProcess.pop();
			F32 inheritedCost = inheritedCosts.front(); inheritedCosts.pop();
			if (curr == BVHNode<Bounds>::NULL_NODE) continue;

			F32 currCost = Bounds{ leafBounds, m_Nodes[curr].NodeBounds }.GetPerimeter();
			F32 currCostInherited = currCost + inheritedCost;
			if (currCostInherited < bestCost)
			{
				bestCost = currCostInherited;
				bestNeighbour = curr;
				// Check if we also need to process children of `curr`.
				F32 newInheritedCost = inheritedCost + currCost - m_Nodes[curr].NodeBounds.GetPerimeter();
				F32 lowerBound = leafBoundsArea + newInheritedCost;
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

	template<typename Bounds>
	void BVHTree2D<Bounds>::ReshapeTree(I32 nodeId)
	{
		I32 currentId = nodeId;
		while (currentId != BVHNode<Bounds>::NULL_NODE)
		{
			I32 leftChild = m_Nodes[currentId].LeftChild;
			I32 rightChild = m_Nodes[currentId].RightChild;
			m_Nodes[currentId].Height = 1 + Math::Max(m_Nodes[leftChild].Height, m_Nodes[rightChild].Height);
			m_Nodes[currentId].NodeBounds = Bounds{ m_Nodes[leftChild].NodeBounds, m_Nodes[rightChild].NodeBounds };
			
			currentId = RebalanceBnB(currentId);

			currentId = m_Nodes[currentId].Parent;
		}
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::Resize(U32 startIndex, U32 endIndex)
	{
		m_Nodes.resize(m_Nodes.size() + (endIndex - startIndex));
		// Create a linked list of free nodes.
		for (U32 i = startIndex; i < endIndex - 1; i++)
		{
			m_Nodes[i].Next = I32(i + 1);
			m_Nodes[i].Height = -1;
		}
		m_Nodes.back().Next = BVHNode<Bounds>::NULL_NODE;
		m_Nodes.back().Height = -1;
	}

	template<typename Bounds>
	I32 BVHTree2D<Bounds>::AllocateNode()
	{
		// If no more free nodes.
		if (m_FreeList == BVHNode<Bounds>::NULL_NODE)
		{
			U32 currentCapacity = U32(m_Nodes.size());
			Resize(currentCapacity, 2 * currentCapacity);
			m_FreeList = I32(currentCapacity);
			m_FreeNodesCount = I32(currentCapacity);
		}

		I32 freeLeaf = m_FreeList;
		m_FreeList = m_Nodes[m_FreeList].Next;
		m_Nodes[freeLeaf].Parent = BVHNode<Bounds>::NULL_NODE;
		m_Nodes[freeLeaf].Height = 0;
		m_FreeNodesCount--;
		return freeLeaf;
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::FreeNode(I32 nodeId)
	{
		m_Nodes[nodeId].Next = m_FreeList;
		m_Nodes[nodeId].Height = -1;
		m_FreeList = nodeId;
		m_FreeNodesCount++;
	}

	template <typename Bounds>
	typename BVHTree2D<Bounds>::TreeRotation BVHTree2D<Bounds>::FindRotation(I32 siblingToChange, I32 siblingAsChange)
	{
		F32 toChangeSA = m_Nodes[siblingToChange].NodeBounds.GetPerimeter();
		// LSA - if we changed left node to `asChange`, RSA otherwise.
		F32 changedLSA = std::numeric_limits<F32>::max(); F32 changedRSA = std::numeric_limits<F32>::max();
		BVHNode<Bounds> toChange = m_Nodes[siblingToChange];
		BVHNode<Bounds> asChange = m_Nodes[siblingAsChange];
		if (toChange.LeftChild != BVHNode<Bounds>::NULL_NODE) changedRSA = Bounds{m_Nodes[toChange.LeftChild].NodeBounds, asChange.NodeBounds}.GetPerimeter();
		if (toChange.RightChild != BVHNode<Bounds>::NULL_NODE) changedLSA = Bounds{m_Nodes[toChange.RightChild].NodeBounds, asChange.NodeBounds}.GetPerimeter();

		if (Math::Min(toChangeSA, Math::Min(changedLSA, changedRSA)) == changedLSA) return TreeRotation{siblingAsChange, toChange.LeftChild, changedLSA};
		if (Math::Min(toChangeSA, Math::Min(changedLSA, changedRSA)) == changedRSA) return TreeRotation{siblingAsChange, toChange.RightChild, changedRSA};
		return {};
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::Rotate(I32 higherI, I32 lowerI)
	{
		BVHNode<Bounds> h = m_Nodes[higherI];
		BVHNode<Bounds> l = m_Nodes[lowerI];

		ENGINE_CORE_ASSERT(h.Parent == m_Nodes[l.Parent].Parent, "Invalid parameters.")
		
		BVHNode<Bounds> hP = m_Nodes[h.Parent];
		BVHNode<Bounds> lP = m_Nodes[l.Parent];
		if (hP.LeftChild == higherI) hP.LeftChild = lowerI;
		else hP.RightChild = lowerI;
		if (lP.LeftChild == lowerI) lP.LeftChild = higherI;
		else lP.RightChild = higherI;
		std::swap(h.Parent, l.Parent);
		lP.NodeBounds = Bounds{m_Nodes[lP.LeftChild].NodeBounds, m_Nodes[lP.RightChild].NodeBounds};
	}

	template <typename Bounds>
	I32 BVHTree2D<Bounds>::RebalanceBnB(I32 nodeId)
	{
		BVHNode<Bounds> a = m_Nodes[nodeId];
		if (a.Height < 2) return nodeId;
		
		BVHNode<Bounds> al = m_Nodes[a.LeftChild];
		BVHNode<Bounds> ar = m_Nodes[a.RightChild];

		auto&& [hA, lB, SAA] = FindRotation(a.RightChild, a.LeftChild);
		auto&& [hB, lA, SAB] = FindRotation(a.LeftChild, a.RightChild);

		if (SAA < SAB) Rotate(hA, lB);
		else if (SAB < SAA) Rotate(hB, lA);

		return nodeId;
	}

	template<typename Bounds>
	F32 BVHTree2D<Bounds>::ComputeTotalCost() const
	{
		F32 cost = 0.0f;
		for (auto& node : m_Nodes)
		{
			if (node.Height > 0) cost += node.NodeBounds.GetPerimeter();
		}
		return cost;
	}
}