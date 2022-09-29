#pragma once

#include "Collider.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

/**
* Heavily inspired by box2d (very heavily). 
* Use callback in query (instead of returning a vector of elements / indices)
* Use enlarged aabbs for less replace operations
* 
* 
*/

namespace Engine
{
	// Holds a pair of rigid bodies, 
	// if broad phase detects a collistion between their Collider2d objects.
	struct PotentialContact
	{
		std::array<RigidBody2D*, 2> Bodies;
	};

	template <typename Bounds>
	struct BVHNode
	{
		static constexpr auto NULL_NODE = -1;
		static constexpr auto NULL_ITEM = -1;
		// Stores enlarged bounds (box2d like).
		Bounds Bounds;
		I32 LeftChild = BVHNode::NULL_NODE;
		I32 RightChild = BVHNode::NULL_NODE;
		// 0 for leaves, -1 if node is free.
		I32 Height = -1;
		// If node is free, stores the index of next free node, 
		// stores parent otherwise.
		union
		{
			I32 Parent;
			I32 Next;
		};
		// Id of item in some external container.
		I32 ItemId = BVHNode::NULL_ITEM;
		bool Moved = true;

		bool IsLeaf() const
		{
			return Height == 0;
		}
	};

	template <typename Bounds>
	class BVHTree
	{
	public:
		BVHTree();
		
		// `itemId` is id of object in some external container,
		// Returns the index of node.
		I32 Insert(I32 itemId, const Bounds& bounds);
		// `nodeId` is returned by Insert().
		void Remove(I32 nodeId);
	private:
		void Resize(U32 startIndex, U32 endIndex);
		void InsertLeaf(I32 leafId);
		I32 FindBestNeighbour(I32 leafId);
		void ReshapeTree(I32 leafId);
		I32 RebalanceTree(I32 nodeId);
		I32 AllocateNode();
		F32 ComputeTotalCost();
	private:
		static constexpr auto s_BoundsGrowth = 0.1f;
		std::vector<BVHNode<Bounds>> m_Nodes;
		I32 m_FreeHead = BVHNode<Bounds>::NULL_NODE;
		I32 m_FreeNodesCount = 0;
		I32 m_TreeRoot = BVHNode<Bounds>::NULL_NODE;
	};

	template<typename Bounds>
	inline BVHTree<Bounds>::BVHTree()
	{
		// Allocate initial buffer for nodes.
		static U32 initialNodesAmount = 16;
		Resize(0, initialNodesAmount);
		m_FreeHead = 0;
		m_FreeNodesCount = m_Nodes.size();
	}

	template<typename Bounds>
	inline I32 BVHTree<Bounds>::Insert(I32 itemId, const Bounds& bounds)
	{
		// Allocate a new leaf.
		I32 leafIndex = AllocateNode();
		m_Nodes[leafIndex].ItemId = itemId;
		m_Nodes[leafIndex].Bounds = bounds;
		// Enlarge aabb so fewer relocation operations is required (box2d-like).
		glm::vec2 boundsExpansion{ s_BoundsGrowth };
		m_Nodes[leafIndex].Bounds.Expand(boundsExpansion);

		InsertLeaf(leafIndex);

		return leafIndex;
	}

	template<typename Bounds>
	inline void BVHTree<Bounds>::Remove(I32 nodeId)
	{

	}

	template<typename Bounds>
	inline void BVHTree<Bounds>::InsertLeaf(I32 leafId)
	{
		// If this is the first insertion (tree is empty).
		if (m_TreeRoot == BVHNode<Bounds>::NULL_NODE)
		{
			m_TreeRoot = leafId;
			return;
		}
		// Else we need to find the best place (neighbour)
		// for the given leaf, based on some cost function.
		
		Bounds& leafBounds = m_Nodes[leafId].Bounds;

		// Find the best neighbour.
		I32 bestNeighbour = FindBestNeighbour(leafId);
		
		// Construct new parent.
		I32 oldParent = m_Nodes[bestNeighbour].Parent;
		I32 newParent = AllocateNode();
		m_Nodes[newParent].Parent = oldParent;
		m_Nodes[newParent].Height = m_Nodes[bestNeighbour].Height + 1;
		m_Nodes[newParent].Bounds = Bounds{ leafBounds, m_Nodes[bestNeighbour].Bounds };
		
		m_Nodes[newParent].LeftChild = leafId;
		m_Nodes[newParent].RightChild = bestNeighbour;
		m_Nodes[leafId].Parent = newParent;
		m_Nodes[bestNeighbour].Parent = newParent;

		if (oldParent != BVHNode<Bounds>::NULL_NODE)
		{
			if (bestNeighbour == m_Nodes[oldParent].LeftChild)
			{
				m_Nodes[newParent].LeftChild = newParent;
			}
			else
			{
				m_Nodes[newParent].RightChild = newParent;
			}
		}
		// if the sibling was the root.
		else
		{
			m_TreeRoot = newParent;
		}
		
		// Reshape the tree.
		ReshapeTree(leafId);
	}

	template<typename Bounds>
	inline I32 BVHTree<Bounds>::FindBestNeighbour(I32 leafId)
	{
		Bounds& leafBounds = m_Nodes[leafId].Bounds;
		// Find the best neighbour.
		I32 currentNode = m_TreeRoot;
		while (m_Nodes[currentNode].IsLeaf() == false)
		{
			I32 leftChild = m_Nodes[currentNode].LeftChild;
			I32 rightChild = m_Nodes[currentNode].RightChild;

			// `area` instead of perimeter to be consistent with 3d case.
			F32 area = m_Nodes[currentNode].Bounds.GetPerimeter();
			Bounds combined = Bounds{ leafBounds, m_Nodes[currentNode].Bounds };
			F32 combinedArea = combined.GetPerimeter();
			// Cost of inserting the leaf to this node.
			// TODO: 2.0 ?
			F32 cost = 2.0f * combinedArea;
			F32 inheritanceCost = 2.0f * (combinedArea - area);

			// Determine the cost of descending to childs.
			F32 costLeft = 0.0f;
			if (m_Nodes[leftChild].IsLeaf())
			{
				Bounds childCombined{ leafBounds, m_Nodes[leftChild].Bounds };
				costLeft = childCombined.GetPerimeter();
			}
			else
			{
				Bounds childCombined{ leafBounds, m_Nodes[leftChild].Bounds };
				F32 oldArea = m_Nodes[leftChild].Bounds.GetPerimeter();
				F32 newArea = childCombined.GetPerimeter();
				costLeft = (newArea - oldArea) + inheritanceCost;
			}

			F32 costRight = 0.0f;
			if (m_Nodes[rightChild].IsLeaf())
			{
				Bounds childCombined{ leafBounds, m_Nodes[rightChild].Bounds };
				costRight = childCombined.GetPerimeter();
			}
			else
			{
				Bounds childCombined{ leafBounds, m_Nodes[rightChild].Bounds };
				F32 oldArea = m_Nodes[rightChild].Bounds.GetPerimeter();
				F32 newArea = childCombined.GetPerimeter();
				costRight = (newArea - oldArea) + inheritanceCost;
			}

			// Early exit.
			if (cost <= costLeft && cost <= costRight) break;
			// Else descend.
			if (costLeft < cost) currentNode = leftChild;
			else currentNode = rightChild;
		}
		return currentNode;
	}

	template<typename Bounds>
	inline void BVHTree<Bounds>::ReshapeTree(I32 leafId)
	{
		I32 currentId = leafId;
		while (currentId != BVHNode<Bounds>::NULL_NODE)
		{
			// TODO: rebalance.

			I32 leftChild = m_Nodes[currentId].LeftChild;
			I32 rightChild = m_Nodes[currentId].RightChild;
			m_Nodes[currentId].Height = 1 + Math::Max(m_Nodes[leftChild].Height, m_Nodes[rightChild].Height);
			m_Nodes[currentId].Bounds = Bounds{ m_Nodes[leftChild].Bounds, m_Nodes[rightChild].Bounds };

			currentId = m_Nodes[currentId].Parent;
		}
	}

	template<typename Bounds>
	inline I32 BVHTree<Bounds>::RebalanceTree(I32 nodeId)
	{
		return I32();
	}

	template<typename Bounds>
	inline void BVHTree<Bounds>::Resize(U32 startIndex, U32 endIndex)
	{
		m_Nodes.resize(m_Nodes.size() + (endIndex - startIndex));
		// Create a linked list of free nodes.
		for (U32 i = startIndex; i < endIndex - 1; i++)
		{
			m_Nodes[i].Next = i + 1;
			m_Nodes[i].Height = -1;
		}
		m_Nodes.back().Next = BVHNode<Bounds>::NULL_NODE;
		m_Nodes.back().Height = -1;
	}

	template<typename Bounds>
	inline I32 BVHTree<Bounds>::AllocateNode()
	{
		// If no more free nodes.
		if (m_FreeHead == BVHNode<Bounds>::NULL_NODE)
		{
			U32 currentCapacity = m_Nodes.size();
			Resize(currentCapacity, 2 * currentCapacity);
			m_FreeHead = currentCapacity;
			m_FreeNodesCount = currentCapacity;
		}

		U32 freeLeaf = m_FreeHead;
		m_Nodes[freeLeaf].Parent = BVHNode<Bounds>::NULL_NODE;
		m_Nodes[freeLeaf].Height = 0;
		m_FreeHead = m_Nodes[m_FreeHead].Next;
		m_FreeNodesCount--;
		return freeLeaf;
	}
	
	template<typename Bounds>
	inline F32 BVHTree<Bounds>::ComputeTotalCost()
	{
		F32 cost = 0.0f;
		for (auto& node : m_Nodes)
		{
			if (node.IsLeaf() == false) cost += node.Bounds.GetPerimeter();
		}
		return cost;
	}

}
