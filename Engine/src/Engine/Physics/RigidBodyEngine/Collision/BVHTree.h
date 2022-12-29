#pragma once

#include "Collider.h"

#include <stack>


namespace Engine
{
	// Defined in Rendering/Drawers/BVHTreeDrawer.h
	class BVHTreeDrawer;
}

namespace Engine::Physics
{
	template <typename Bounds>
	struct BVHNode
	{
		static constexpr auto NULL_NODE = -1;
		static constexpr auto NULL_ITEM = -1;
		// Stores enlarged bounds (box2d like).
		Bounds Bounds;
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
		const Bounds& GetBounds(I32 nodeId) const { return m_Nodes[nodeId].Bounds; }

		F32 ComputeTotalCost() const;
	private:
		void Resize(U32 startIndex, U32 endIndex);
		void InsertLeaf(I32 leafId);
		void RemoveLeaf(I32 leafId);
		I32 FindBestNeighbour(I32 leafId);
		void ReshapeTree(I32 nodeId);
		I32 RebalanceTree(I32 nodeId);
		I32 AllocateNode();
		void FreeNode(I32 nodeId);
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
			if (m_Nodes[currentNode].Bounds.Intersects(bounds))
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
		m_Nodes[leafIndex].Bounds = bounds;
		// Enlarge aabb so fewer relocation operations is required (box2d-like).
		glm::vec2 boundsExpansion{ s_BoundsGrowth };
		m_Nodes[leafIndex].Bounds.Expand(boundsExpansion);

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
		I32 bestNeighbour = FindBestNeighbour(leafId);

		// Construct new Parent.
		I32 oldParent = m_Nodes[bestNeighbour].Parent;
		I32 newParent = AllocateNode();
		m_Nodes[newParent].Parent = oldParent;
		m_Nodes[newParent].Height = m_Nodes[bestNeighbour].Height + 1;
		const Bounds& leafBounds = m_Nodes[leafId].Bounds;
		m_Nodes[newParent].Bounds = Bounds{ leafBounds, m_Nodes[bestNeighbour].Bounds };
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
		I32 neighbour = BVHNode<Bounds>::NULL_NODE;
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

		const Bounds& treeBounds = m_Nodes[nodeId].Bounds;
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
		m_Nodes[nodeId].Bounds = expandedBounds;
		InsertLeaf(nodeId);
		m_Nodes[nodeId].Moved = true;

		return true;
	}

	template<typename Bounds>
	I32 BVHTree2D<Bounds>::FindBestNeighbour(I32 leafId)
	{
		const Bounds& leafBounds = m_Nodes[leafId].Bounds;
		// Find the best neighbour.
		I32 currentNode = m_TreeRoot;
		while (m_Nodes[currentNode].IsLeaf() == false)
		{
			I32 leftChild = m_Nodes[currentNode].LeftChild;
			I32 rightChild = m_Nodes[currentNode].RightChild;

			// `area` instead of perimeter to be consistent with 3d case.
			F32 area = m_Nodes[currentNode].Bounds.GetPerimeter();
			// Cost of inserting the leaf to this node.
			//? 2.0 ?
			F32 costThreshold = 2.0f * area;

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
				costLeft = (newArea - oldArea);
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
				costRight = (newArea - oldArea);
			}

			// Early exit.
			if (costThreshold < costLeft && costThreshold < costRight) break;
			// Else descend.
			if (costLeft < costRight) currentNode = leftChild;
			else currentNode = rightChild;
		}
		return currentNode;
	}

	template<typename Bounds>
	void BVHTree2D<Bounds>::ReshapeTree(I32 nodeId)
	{
		I32 currentId = nodeId;
		while (currentId != BVHNode<Bounds>::NULL_NODE)
		{
			currentId = RebalanceTree(currentId);

			I32 leftChild = m_Nodes[currentId].LeftChild;
			I32 rightChild = m_Nodes[currentId].RightChild;
			m_Nodes[currentId].Height = 1 + Math::Max(m_Nodes[leftChild].Height, m_Nodes[rightChild].Height);
			m_Nodes[currentId].Bounds = Bounds{ m_Nodes[leftChild].Bounds, m_Nodes[rightChild].Bounds };

			currentId = m_Nodes[currentId].Parent;
		}
	}

	// Straight from box2d.
	// TODO: balance based on sah not on height.
	template<typename Bounds>
	I32 BVHTree2D<Bounds>::RebalanceTree(I32 nodeId)
	{
		I32 AId = nodeId;
		BVHNode<Bounds>& A = m_Nodes[AId];
		if (A.IsLeaf() || A.Height < 2) return AId;

		I32 BId = A.LeftChild;
		I32 CId = A.RightChild;
		BVHNode<Bounds>& B = m_Nodes[BId];
		BVHNode<Bounds>& C = m_Nodes[CId];

		// In a perfect world balance is 0, but this is not a perfect world.
		I32 balance = C.Height - B.Height;
		if (balance > 1)
		{
			// Rotate right child up.
			I32 FId = C.LeftChild;
			I32 GId = C.RightChild;
			BVHNode<Bounds>& F = m_Nodes[FId];
			BVHNode<Bounds>& G = m_Nodes[GId];

			// Swap A and C.
			C.LeftChild = AId;
			C.Parent = A.Parent;
			A.Parent = CId;

			// Old parent of A should point to C.
			if (C.Parent != BVHNode<Bounds>::NULL_NODE)
			{
				if (m_Nodes[C.Parent].LeftChild == AId)
				{
					m_Nodes[C.Parent].LeftChild = CId;
				}
				else
				{
					m_Nodes[C.Parent].RightChild = CId;
				}
			}
			else
			{
				m_TreeRoot = CId;
			}

			// Rotate.
			if (F.Height > G.Height)
			{
				C.RightChild = FId;
				A.RightChild = GId;
				G.Parent = AId;
				A.Bounds = Bounds{ B.Bounds, G.Bounds };
				C.Bounds = Bounds{ A.Bounds, F.Bounds };
				A.Height = 1 + Math::Max(B.Height, G.Height);
				C.Height = 1 + Math::Max(A.Height, F.Height);
			}
			else
			{
				C.RightChild = GId;
				A.RightChild = FId;
				F.Parent = AId;
				A.Bounds = Bounds{ B.Bounds, F.Bounds };
				C.Bounds = Bounds{ A.Bounds, G.Bounds };
				A.Height = 1 + Math::Max(B.Height, F.Height);
				C.Height = 1 + Math::Max(A.Height, G.Height);
			}
			return CId;
		}


		else if (balance < -1)
		{
			// Rotate left child up.
			I32 DId = B.LeftChild;
			I32 EId = B.RightChild;
			BVHNode<Bounds>& D = m_Nodes[DId];
			BVHNode<Bounds>& E = m_Nodes[EId];

			// Swap A and C.
			B.LeftChild = AId;
			B.Parent = A.Parent;
			A.Parent = BId;

			// Old parent of A should point to B.
			if (B.Parent != BVHNode<Bounds>::NULL_NODE)
			{
				if (m_Nodes[B.Parent].LeftChild == AId)
				{
					m_Nodes[B.Parent].LeftChild = BId;
				}
				else
				{
					m_Nodes[B.Parent].RightChild = BId;
				}
			}
			else
			{
				m_TreeRoot = BId;
			}

			// Rotate.
			if (D.Height > E.Height)
			{
				B.RightChild = DId;
				A.LeftChild = EId;
				E.Parent = AId;
				A.Bounds = Bounds{ C.Bounds, E.Bounds };
				B.Bounds = Bounds{ A.Bounds, D.Bounds };
				A.Height = 1 + Math::Max(C.Height, E.Height);
				B.Height = 1 + Math::Max(A.Height, D.Height);
			}
			else
			{
				B.RightChild = EId;
				A.LeftChild = DId;
				D.Parent = AId;
				A.Bounds = Bounds{ C.Bounds, D.Bounds };
				B.Bounds = Bounds{ A.Bounds, E.Bounds };
				A.Height = 1 + Math::Max(C.Height, D.Height);
				B.Height = 1 + Math::Max(A.Height, E.Height);
			}
			return BId;
		}

		return AId;
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

		U32 freeLeaf = m_FreeList;
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

	template<typename Bounds>
	F32 BVHTree2D<Bounds>::ComputeTotalCost() const
	{
		F32 cost = 0.0f;
		for (auto& node : m_Nodes)
		{
			if (node.Height > 0) cost += node.Bounds.GetPerimeter();
		}
		return cost;
	}
}