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

	struct PotentialContactNode2D
	{
		PotentialContact2D Contact = { { nullptr, nullptr }, {-1, -1} };
		PotentialContactNode2D* Next = nullptr;
		PotentialContactNode2D* Prev = nullptr;
	};

	// TODO: find a better name?
	template <typename Bounds = Engine::DefaultBounds2D>
	class BroadPhase2D
	{
	public:
		~BroadPhase2D();
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

		// Get user payload stored in bvh.
		void* GetPayload(I32 nodeId) const;

		const PotentialContactNode2D* GetContacts() const;
		U32 GetNumberOfContacts() const { return m_NumberOfContacts; }

		void RemoveContact(const PotentialContact2D& contact);

	private:
		PotentialContactNode2D* AddContact(const PotentialContact2D& contact);
		void QueryCallback(I32 otherNode);
	private:
		BVHTree2D<Bounds> m_Tree;

		// Since only the moving objects can generate contacts.
		std::vector<I32> m_MovingBodies;

		PotentialContactNode2D* m_ContactNodes = nullptr;
		std::unordered_map<RigidBody2D*, std::vector<PotentialContactNode2D*>> m_ContactsMap;
		U32 m_NumberOfContacts = 0;

		// Node that may form a pair.
		I32 m_CurrentlyTestedNode = BVHNode<Bounds>::NULL_NODE;
	};

	template<typename Bounds>
	inline BroadPhase2D<Bounds>::~BroadPhase2D()
	{
		PotentialContactNode2D* currentNode = m_ContactNodes;
		while (currentNode != nullptr)
		{
			PotentialContactNode2D* next = currentNode->Next;
			Delete<PotentialContactNode2D>(currentNode);
			currentNode = next;
		}
	}

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
	inline void* BroadPhase2D<Bounds>::GetPayload(I32 nodeId) const
	{
		return m_Tree.GetPayload(nodeId);
	}

	template<typename Bounds>
	inline const PotentialContactNode2D* BroadPhase2D<Bounds>::GetContacts() const
	{
		// We remove contacts as need removing,
		// return contacts as need returning,
		// and we kill 'em as need killing.
		return m_ContactNodes;
	}

	template<typename Bounds>
	inline void BroadPhase2D<Bounds>::RemoveContact(const PotentialContact2D& contact)
	{
		RigidBody2D* primaryBody = contact.Bodies[0];
		RigidBody2D* otherBody = contact.Bodies[1];
		PotentialContactNode2D* toDelete = nullptr;
		auto it = m_ContactsMap[primaryBody].begin();
		while (it != m_ContactsMap[primaryBody].end())
		{
			if (&(*it)->Contact == &contact)
			{
				toDelete = *it;
				break;
			}
			it++;
		}
		// Remove from map.
		m_ContactsMap[primaryBody].erase(it);
		ENGINE_ASSERT(toDelete != nullptr, "Trying to delete unknown contact");
		// Delete node.
		if (toDelete == m_ContactNodes)
		{
			m_ContactNodes = toDelete->Next;
		}
		if (toDelete->Prev != nullptr)
		{
			toDelete->Prev->Next = toDelete->Next;
		}
		if (toDelete->Next != nullptr)
		{
			toDelete->Next->Prev = toDelete->Prev;
		}
		Delete<PotentialContactNode2D>(toDelete);
		m_NumberOfContacts--;
	}

	template<typename Bounds>
	inline PotentialContactNode2D* BroadPhase2D<Bounds>::AddContact(const PotentialContact2D& contact)
	{
		// Allocate new node.
		PotentialContactNode2D* newNode = New<PotentialContactNode2D>();
		newNode->Contact = contact;
		// Insert node to the list.
		newNode->Next = m_ContactNodes;
		if (m_ContactNodes != nullptr)
		{
			m_ContactNodes->Prev = newNode;
		}
		m_ContactNodes = newNode;
		m_NumberOfContacts++;
		return newNode;
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
		for (auto& contactNode : m_ContactsMap[primaryBody])
		{
			if (contactNode->Contact.Bodies[1] == otherBody)
			{
				return;
			}
		}
		for (auto& contactNode : m_ContactsMap[otherBody])
		{
			if (contactNode->Contact.Bodies[1] == primaryBody)
			{
				return;
			}
		}
		PotentialContactNode2D* newNode = AddContact({ {primaryBody, otherBody},{ m_CurrentlyTestedNode, otherNode } });
		m_ContactsMap[primaryBody].push_back(newNode);
	}

}
