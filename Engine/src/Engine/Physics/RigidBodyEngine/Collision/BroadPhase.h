#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "BVHTree.h"

#include <array>

/*
* Use enlarged aabbs for less replace operations
*/

namespace Engine::Physics
{
	// Holds a pair of colliders, whose bounds are colliding.
	struct PotentialContact2D
	{
		std::array<Collider2D*, 2> Colliders;
		std::array<I32, 2> NodeIds;
	};

	template <typename Bounds = DefaultBounds2D>
	class BroadPhase2D
	{
	public:
		// Returns the undelying tree structure.
		const BVHTree2D<Bounds> GetBVHTree() const { return m_Tree; }
		
		// Performs QueryCallback on nodes, that intersects with `bounds`.
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

		std::unordered_map<I32, std::vector<I32>> m_ContactsMap;
		U32 m_NumberOfContacts = 0;

		// Node that may form a pair.
		I32 m_CurrentlyTestedNode = BVHNode<Bounds>::NULL_NODE;
	};


	template<typename Bounds>
	template <typename Callback>
	inline void BroadPhase2D<Bounds>::Query(const Bounds& bounds, Callback callback)
	{
		m_Tree.Query([&callback, this](I32 nodeId) { return this->QueryCallback(nodeId, callback); }, bounds);
	}

	template<typename Bounds>
	inline I32 BroadPhase2D<Bounds>::InsertCollider(Collider2D* collider, const Bounds& bounds)
	{
		I32 nodeID = m_Tree.Insert(static_cast<void*>(collider), bounds);
		m_MovingBodies.push_back(nodeID);
		return nodeID;
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::RemoveCollider(I32 nodeId)
	{
		m_Tree.Pop(nodeId);
	}

	template<typename Bounds>
	inline bool BroadPhase2D<Bounds>::MoveCollider(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity)
	{
		bool hasMoved = m_Tree.Move(nodeId, bounds, velocity);
		if (hasMoved) m_MovingBodies.push_back(nodeId);
		return hasMoved;
	}

	template<typename Bounds>
	inline bool BroadPhase2D<Bounds>::CheckCollision(I32 firstNodeId, I32 secondNodeId)
	{
		return m_Tree.GetBounds(firstNodeId).Intersects(m_Tree.GetBounds(secondNodeId));
	}

	template<typename Bounds>
	template <typename Callback>
	inline void BroadPhase2D<Bounds>::FindContacts(Callback callback)
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
	inline void* BroadPhase2D<Bounds>::GetPayload(I32 nodeId) const
	{
		return m_Tree.GetPayload(nodeId);
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::RemoveContact(const PotentialContact2D& contact)
	{
		
		auto it = m_ContactsMap[contact.NodeIds[0]].begin();
		while (it != m_ContactsMap[contact.NodeIds[0]].end())
		{
			if (*it == contact.NodeIds[1])
			{
				break;
			}
			it++;
		}
		
		// Remove from map.
		m_ContactsMap[contact.NodeIds[0]].erase(it);
		m_NumberOfContacts--;
	}

	template<typename Bounds>
	template <typename Callback>
	inline void BroadPhase2D<Bounds>::QueryCallback(I32 otherNode, Callback callback)
	{
		if (otherNode == m_CurrentlyTestedNode) return;
		// `otherNode` will later become m_CurrentlyTestedNode, so we don't record it twice.
		if (m_Tree.IsMoved(otherNode) && otherNode > m_CurrentlyTestedNode) return;

		// TODO: maybe find a better way.
		Collider2D* primaryCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(m_CurrentlyTestedNode));
		Collider2D* otherCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(otherNode));
		// No contact resolution between colliders of same body.
		if (primaryCollider->GetAttachedRigidBody() == otherCollider->GetAttachedRigidBody()) return;

		for (auto& contactNodeId : m_ContactsMap[m_CurrentlyTestedNode])
		{
			if (contactNodeId == otherNode)
			{
				return;
			}
		}
		for (auto& contactNodeId : m_ContactsMap[otherNode])
		{
			if (contactNodeId == m_CurrentlyTestedNode)
			{
				return;
			}
		}
		PotentialContact2D contact{ {primaryCollider, otherCollider},{ m_CurrentlyTestedNode, otherNode } };
		callback(contact);
		m_ContactsMap[m_CurrentlyTestedNode].push_back(otherNode);
	}	
}