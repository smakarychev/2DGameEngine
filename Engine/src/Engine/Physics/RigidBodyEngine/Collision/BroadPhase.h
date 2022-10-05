#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "BVHTree.h"
#include "Engine/Common/FreeList.h"

#include <array>

/**
* 
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
	struct PotentialContact2D
	{
		std::array<RigidBody2D*, 2> Bodies;
		std::array<I32, 2> NodeIds;
	};

	// TODO: find a better name?
	template <typename Bounds = Engine::DefaultBounds2D>
	class BroadPhase2D
	{
	public:
		// Returns the undelying tree structure.
		const BVHTree2D<Bounds> GetBVHTree() const { return m_Tree; }
		
		// Performs QueryCallback on nodes, that intersects with `bounds`.
		void Query(const Bounds& bounds);

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
		void FindContacts();

		const std::vector<PotentialContact2D>& GetContacts();

		void RemoveContact(const PotentialContact2D& contact);

	private:
		void QueryCallback(I32 otherNode);
	private:
		BVHTree2D<Bounds> m_Tree;

		// Since only the moving objects can generate contacts.
		std::vector<I32> m_MovingBodies;

		std::vector<PotentialContact2D> m_Contacts;

		struct ContactMapElement
		{
			RigidBody2D* Body;
			I32 ContactId;

		};
		std::unordered_map<RigidBody2D*, std::vector<ContactMapElement>> m_ContactsMap;

		// Node that may form a pair.
		I32 m_CurrentlyTestedNode = BVHNode<Bounds>::NULL_NODE;
	};

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::Query(const Bounds& bounds)
	{
		m_Tree.Query(BIND_FN(BroadPhase2D::QueryCallback), bounds);
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
	inline void BroadPhase2D<Bounds>::FindContacts()
	{
		// Generate pairs.
		for (auto& movingId : m_MovingBodies)
		{
			m_CurrentlyTestedNode = movingId;
			const Bounds& movingBounds = m_Tree.GetBounds(movingId);
			Query(movingBounds);
		}

		// Clear all moving objects at the end.
		for (auto& movingId : m_MovingBodies)
		{
			m_Tree.ResetMoved(movingId);
		}
		m_MovingBodies.clear();
	}

	template<typename Bounds>
	inline const std::vector<PotentialContact2D>& BroadPhase2D<Bounds>::GetContacts()
	{
		// We remove contacts as need removing,
		// return contacts as need returning,
		// and we kill 'em as need killing.
		
		std::erase_if(m_Contacts, [](auto& contact) { return contact.NodeIds[0] == -1 && contact.NodeIds[1] == -1; });
		return m_Contacts;
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::RemoveContact(const PotentialContact2D& contact)
	{
		RigidBody2D* primaryBody = contact.Bodies[0];
		RigidBody2D* otherBody = contact.Bodies[1];
		U32 index = 0;
		auto it = m_ContactsMap[primaryBody].begin();
		while (it != m_ContactsMap[primaryBody].end())
		{
			if (it->Body == otherBody)
			{
				index = it->ContactId;
				break;
			}
			it++;
		}
		// Remove from map.
		m_ContactsMap[primaryBody].erase(it);
		// Mark contact as deleted.
		m_Contacts[index].NodeIds[0] = m_Contacts[index].NodeIds[1] = -1;
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::QueryCallback(I32 otherNode)
	{
		if (otherNode == m_CurrentlyTestedNode) return;
		// `otherNode` will later become m_CurrentlyTestedNode, so we don't record it twice.
		if (m_Tree.IsMoved(otherNode) && otherNode > m_CurrentlyTestedNode) return;

		// TODO: maybe find a better way.
		RigidBody2D* primaryBody = reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(m_CurrentlyTestedNode));
		RigidBody2D* otherBody = reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(otherNode));
		bool alreadyExists = false;
		for (auto& body : m_ContactsMap[primaryBody])
		{
			if (body.Body == otherBody)
			{
				alreadyExists = true; 
				return;
			}
		}
		for (auto& body : m_ContactsMap[otherBody])
		{
			if (body.Body == primaryBody)
			{
				alreadyExists = true;
				return;
			}
		}

		m_Contacts.push_back({ primaryBody, otherBody, m_CurrentlyTestedNode, otherNode });
		m_ContactsMap[primaryBody].push_back(ContactMapElement{ otherBody, static_cast<I32>(m_Contacts.size() - 1) });
	}

}
