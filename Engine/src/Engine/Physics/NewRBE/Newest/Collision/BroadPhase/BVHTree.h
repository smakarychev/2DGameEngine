#pragma once

#include "../Colliders/Collider2D.h"

#include <stack>

#include "Engine/Physics/NewRBE/Newest/Collision/CollisionLayer.h"

namespace Engine
{
	class BVHTreeDrawer;
}

namespace Engine::WIP::Physics::Newest
{
	struct BVHNode
	{
		static constexpr auto NULL_NODE = std::numeric_limits<U32>::max();
		static constexpr auto NULL_ITEM = std::numeric_limits<U32>::max();
		// Stores enlarged AABB2D (box2d like).
		AABB2D NodeBounds;
		U32 LeftChild = NULL_NODE;
		U32 RightChild = NULL_NODE;
		// 0 for leaves, -1 if node is free.
		I32 Height = -1;
		// If node is free, stores the index of next free node, 
		// stores parent otherwise.
		union
		{
			U32 Parent;
			U32 Next = NULL_NODE;
		};
		// It is assumed that Payload is of type Collider2D.
		void* Payload = nullptr;
		bool Moved = false;

		bool IsLeaf() const
		{
			return Height == 0;
		}
	};
	
	class BVHTree2D
	{
		friend class ::Engine::BVHTreeDrawer;
	public:
		struct TreeRotation
		{
			U32 High = BVHNode::NULL_NODE;
			U32 Low = BVHNode::NULL_NODE;
			F32 SA = std::numeric_limits<F32>::max();
		};
	public:
		BVHTree2D();
		void SetCollisionLayer(CollisionLayer layer) { m_CollisionLayer = layer; }
		CollisionLayer GetCollisionLayer() const { return m_CollisionLayer; }
		void Clear();
		// `itemId` is id of object in some external container,
		// Returns the index of node.
		U32 Insert(void* payload, const AABB2D& bounds);

		// `nodeId` is returned by Insert().
		void Remove(U32 nodeId);

		// `nodeId` is returned by Insert(), `velocity` here is a 
		// mere measure of displacement, not strictly related 
		// to physical velocity of the object.
		bool Move(U32 nodeId, const AABB2D& bounds, const glm::vec2& velocity);

		template <typename Callback>
		void Query(const Callback& callback, const AABB2D& bounds) const;

		bool IsMoved(U32 nodeId) const { return m_Nodes[nodeId].Moved; }
		void ResetMoved(U32 nodeId) { m_Nodes[nodeId].Moved = false; }
		void SetMoved(U32 nodeId) { m_Nodes[nodeId].Moved = true; }
		void* GetPayload(U32 nodeId) const { return m_Nodes[nodeId].Payload; }
		const AABB2D& GetAABB2D(U32 nodeId) const { return m_Nodes[nodeId].NodeBounds; }

		F32 ComputeTotalCost() const;
	private:
		void Resize(U32 startIndex, U32 endIndex);
		void InsertLeaf(U32 leafId);
		void RemoveLeaf(U32 leafId);
		U32 FindBestNeighbourBnB(U32 leafId);
		void ReshapeTree(U32 nodeId);
		U32 AllocateNode();
		void FreeNode(U32 nodeId);
		TreeRotation FindRotation(U32 siblingToChange, U32 siblingAsChange);
		void Rotate(U32 higherI, U32 lowerI);
		U32 RebalanceBnB(U32 nodeId);
	private:
		// Used to enlarge AABB2D so fewer reinsertions performed.
		static constexpr auto s_AABB2DGrowth = 0.1f;
		// Used to greatly enlarge AABB2D of rapidly moving objects.
		static constexpr auto s_DynamicAABB2DGrowth = 4.0f;
		
		CollisionLayer m_CollisionLayer{};
		
		std::vector<BVHNode> m_Nodes;
		U32 m_FreeList = BVHNode::NULL_NODE;
		U32 m_FreeNodesCount = 0;
		U32 m_TreeRoot = BVHNode::NULL_NODE;
	};

	template<typename Callback>
	void BVHTree2D::Query(const Callback& callback, const AABB2D& bounds) const
	{
		std::stack<U32> toProcess;
		toProcess.push(m_TreeRoot);

		while (toProcess.empty() == false)
		{
			U32 currentNode = toProcess.top(); toProcess.pop();
			if (currentNode == BVHNode::NULL_NODE)
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
}
