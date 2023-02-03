#pragma once

#include "Engine/Physics/NewRBE/RigidBody.h"
#include "BVHTree.h"

#include <array>

namespace Engine::WIP::Physics
{
	// Holds a pair of colliders, whose bounds are colliding.
	struct PotentialContact2D
	{
		std::array<Collider2D*, 2> Colliders;
	};

	template <typename Bounds = DefaultBounds2D>
	class BroadPhase2D
	{
	public:
		// Returns the underlying tree structure.
		const BVHTree2D<Bounds>& GetBVHTree() const { return m_Tree; }
		BVHTree2D<Bounds>& GetBVHTree() { return m_Tree; }
		void Clear();
		
		// Performs Callback on nodes, that intersects with `bounds`.
		template <typename Callback>
		void Query(const Bounds& bounds, Callback callback);

		I32 InsertCollider(Collider2D* collider, const Bounds& bounds);
		
		// `nodeId` is returned by InsertCollider().
		void RemoveCollider(I32 nodeId);
		
		// `nodeId` is returned by InsertCollider(), `velocity` here is a 
		// mere measure of displacement, not strictly related 
		// to the physical velocity of the object.
		bool MoveCollider(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity);

		// Checks for bounds collision.
		bool CheckCollision(I32 firstNodeId, I32 secondNodeId);

		// Find all contacts (pairs of objects, that possibly collide).
		template <typename Callback>
		void FindContacts(Callback callback);

		// Get user payload stored in bvh.
		void* GetPayload(I32 nodeId) const;

		U32 GetNumberOfContacts() const { return m_NumberOfContacts; }

		void RemoveContact(const PotentialContact2D& contact);

		template <typename Callback>
		void QueryCallback(I32 otherNode, Callback callback);
	private:
		BVHTree2D<Bounds> m_Tree;

		// Since only the moving objects can generate contacts.
		std::vector<I32> m_MovingBodies;

		std::unordered_map<I64, bool> m_ContactsMap;
		U32 m_NumberOfContacts = 0;

		// Node that may form a pair.
		I32 m_CurrentlyTestedNode = BVHNode<Bounds>::NULL_NODE;
	};


	template<typename Bounds>
	template <typename Callback>
	void BroadPhase2D<Bounds>::Query(const Bounds& bounds, Callback callback)
	{
		m_Tree.Query([&callback, this](I32 nodeId) { return this->QueryCallback(nodeId, callback); }, bounds);
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::Clear()
	{
		m_MovingBodies.clear();
		m_ContactsMap.clear();
		m_Tree.Clear();
	}

	template<typename Bounds>
	I32 BroadPhase2D<Bounds>::InsertCollider(Collider2D* collider, const Bounds& bounds)
	{
		I32 nodeID = m_Tree.Insert(static_cast<void*>(collider), bounds);
		m_MovingBodies.push_back(nodeID);
		return nodeID;
	}

	template <typename Bounds>
	void BroadPhase2D<Bounds>::RemoveCollider(I32 nodeId)
	{
		m_Tree.Remove(nodeId);
	}

	template<typename Bounds>
	bool BroadPhase2D<Bounds>::MoveCollider(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity)
	{
		bool hasMoved = m_Tree.Move(nodeId, bounds, velocity);
		if (hasMoved) m_MovingBodies.push_back(nodeId);
		return hasMoved;
	}

	template<typename Bounds>
	bool BroadPhase2D<Bounds>::CheckCollision(I32 firstNodeId, I32 secondNodeId)
	{
		return m_Tree.GetBounds(firstNodeId).Intersects(m_Tree.GetBounds(secondNodeId));
	}

	template<typename Bounds>
	template <typename Callback>
	void BroadPhase2D<Bounds>::FindContacts(Callback callback)
	{
		// Generate pairs.
		for (auto& movingId : m_MovingBodies)
		{
			m_CurrentlyTestedNode = movingId;
			const Bounds& movingBounds = m_Tree.GetBounds(movingId);
			Query(movingBounds, callback);
		}

		// Clear all moving objects at the end.
		for (auto& movingId : m_MovingBodies)
		{
			m_Tree.ResetMoved(movingId);
		}
		m_MovingBodies.clear();
	}

	template<typename Bounds>
	void* BroadPhase2D<Bounds>::GetPayload(I32 nodeId) const
	{
		return m_Tree.GetPayload(nodeId);
	}

	template<typename Bounds>
	void BroadPhase2D<Bounds>::RemoveContact(const PotentialContact2D& contact)
	{
		I32 nodeA = contact.Colliders[0]->GetBroadPhaseNode();
		I32 nodeB = contact.Colliders[1]->GetBroadPhaseNode();
		I32 primaryNode = Math::Min(nodeA, nodeB);
		I32 secondaryNode = Math::Max(nodeA, nodeB);
		m_ContactsMap.erase(m_ContactsMap.find(Math::I32PairKey(std::make_pair(primaryNode, secondaryNode))));
		m_NumberOfContacts--;
	}

	template<typename Bounds>
	template <typename Callback>
	void BroadPhase2D<Bounds>::QueryCallback(I32 otherNode, Callback callback)
	{
		if (otherNode == m_CurrentlyTestedNode) return;
		// `otherNode` will later become m_CurrentlyTestedNode, so we don't record it twice.
		if (m_Tree.IsMoved(otherNode) && otherNode < m_CurrentlyTestedNode) return;

		// TODO: maybe find a better way.
		I32 primaryNode = Math::Min(m_CurrentlyTestedNode, otherNode);
		I32 secondaryNode = Math::Max(m_CurrentlyTestedNode, otherNode);
		Collider2D* primaryCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(primaryNode));
		Collider2D* secondaryCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(secondaryNode));
		// No contact resolution between colliders of the same body.
		if (primaryCollider->GetAttachedRigidBody() == secondaryCollider->GetAttachedRigidBody()) return;
		I64 mapKey = Math::I32PairKey(std::make_pair(primaryNode, secondaryNode));
		if (m_ContactsMap.find(mapKey) != m_ContactsMap.end()) return;		
		
		PotentialContact2D contact{ {primaryCollider, secondaryCollider} };
		callback(contact);
		m_ContactsMap[mapKey] = true;
	}
}
