#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "BVHTree.h"

#include <array>

/*
* Use enlarged aabbs for less replace operations
*/

namespace Engine
{
	// Holds a pair of rigid bodies, 
	// if broad phase detects a collistion between their Collider2d objects.
	struct PotentialContact2D
	{
		std::array<RigidBody2D*, 2> Bodies;
		std::array<I32, 2> NodeIds;
	};

	template <typename Bounds = Engine::DefaultBounds2D>
	class BroadPhase2D
	{
	public:
		// Returns the undelying tree structure.
		const BVHTree2D<Bounds> GetBVHTree() const { return m_Tree; }
		
		// Performs QueryCallback on nodes, that intersects with `bounds`.
		template <typename Callback>
		void Query(const Bounds& bounds, Callback callback);

		I32 InsertRigidBody(RigidBody2D* body, const Bounds& bounds);
		
		// `nodeId` is returned by InsertRigidBody().
		void RemoveRigidBody(I32 nodeId);
		
		// `nodeId` is returned by InsertRigidBody(), `velocity` here is a 
		// mere measure of displacement, not strictly related 
		// to the physical velocity of the object.
		bool MoveRigidBody(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity);

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
	inline I32 BroadPhase2D<Bounds>::InsertRigidBody(RigidBody2D* body, const Bounds& bounds)
	{
		I32 nodeID = m_Tree.Insert(static_cast<void*>(body), bounds);
		m_MovingBodies.push_back(nodeID);
		return nodeID;
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::RemoveRigidBody(I32 nodeId)
	{
		m_Tree.Remove(nodeId);
	}

	template<typename Bounds>
	inline bool BroadPhase2D<Bounds>::MoveRigidBody(I32 nodeId, const Bounds& bounds, const glm::vec2& velocity)
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
		RigidBody2D* primaryBody = reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(m_CurrentlyTestedNode));
		RigidBody2D* otherBody = reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(otherNode));
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
		PotentialContact2D contact{ {primaryBody, otherBody},{ m_CurrentlyTestedNode, otherNode } };
		callback(contact);
		m_ContactsMap[m_CurrentlyTestedNode].push_back(otherNode);
	}

}
