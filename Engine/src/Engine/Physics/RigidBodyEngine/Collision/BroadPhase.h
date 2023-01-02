#pragma once

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "BVHTree.h"

#include <array>

/*glm::vec2 ditstMAMax = ifrom.Contacts[1inedexOfMaxdepth].
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
		const BVHTree2D<Bounds>& GetBVHTree() const { return m_Tree; }
		BVHTree2D<Bounds>& GetBVHTree() { return m_Tree; }
		void Clear();
		
		// Performs Callback on nodes, that intersects with `bounds`.
		template <typename Callback>
		void Query(const Bounds& bounds, Callback callback);

		I32 InsertCollider(Collider2D* collider, const Bounds& bounds);
		
		// `nodeId` is returned by InsertCollider().
		// Performs Callback for each contact that node had.
		template <typename Callback>
		void RemoveCollider(I32 nodeId, Callback callback);
		
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
		template <typename Callback>
		void RemoveNodeContacts(I32 nodeId, Callback callback);
	private:
		BVHTree2D<Bounds> m_Tree;

		// Since only the moving objects can generate contacts.
		std::vector<I32> m_MovingBodies;

		std::unordered_map<I32, std::vector<I32>> m_ContactsMap;
		std::unordered_map<U64, bool> m_PrimalityMap;
		std::unordered_map<U64, void*> m_BroadToNarrowContactMap;
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
		m_PrimalityMap.clear();
		m_BroadToNarrowContactMap.clear();
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
	template <typename Callback>
	void BroadPhase2D<Bounds>::RemoveCollider(I32 nodeId, Callback callback)
	{
		RemoveNodeContacts(nodeId, callback);
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
		auto it1 = std::ranges::find(m_ContactsMap[contact.NodeIds[0]], contact.NodeIds[1]);
		auto it0 = std::ranges::find(m_ContactsMap[contact.NodeIds[1]], contact.NodeIds[0]);
		// Remove from map.
		m_ContactsMap[contact.NodeIds[0]].erase(it1);
		m_ContactsMap[contact.NodeIds[1]].erase(it0);
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
		Collider2D* primaryCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(m_CurrentlyTestedNode));
		Collider2D* otherCollider = reinterpret_cast<Collider2D*>(m_Tree.GetPayload(otherNode));
		// No contact resolution between colliders of the same body.
		if (primaryCollider->GetAttachedRigidBody() == otherCollider->GetAttachedRigidBody() && primaryCollider->GetAttachedRigidBody() != nullptr) return;

		auto& curContacts = m_ContactsMap[m_CurrentlyTestedNode];
		auto& otherContacts = m_ContactsMap[otherNode];
		
		if (std::ranges::find(curContacts, otherNode) != curContacts.end()) return;
		if (std::ranges::find(otherContacts, m_CurrentlyTestedNode) != otherContacts.end()) return;

		PotentialContact2D contact{ {primaryCollider, otherCollider},{ m_CurrentlyTestedNode, otherNode } };
		void* contactAddress = callback(contact);
		m_BroadToNarrowContactMap[Math::I32PairKey(std::make_pair(m_CurrentlyTestedNode, otherNode))] = contactAddress;
		m_ContactsMap[m_CurrentlyTestedNode].push_back(otherNode);
		m_ContactsMap[otherNode].push_back(m_CurrentlyTestedNode);
		m_PrimalityMap[Math::I32PairKey(std::make_pair(m_CurrentlyTestedNode, otherNode))] = true;
		m_PrimalityMap[Math::I32PairKey(std::make_pair(otherNode, m_CurrentlyTestedNode))] = false;
	}

	template <typename Bounds>
	template <typename Callback>
	void BroadPhase2D<Bounds>::RemoveNodeContacts(I32 nodeId, Callback callback)
	{
		// Iterate over all nodes that `nodeId` has contact with,
		// and delete those contacts first if needed.
		for (auto& otherNode : m_ContactsMap[nodeId])
		{
			auto nodesPair = std::make_pair(otherNode, nodeId);
			auto nodesPairReverse = std::make_pair(nodeId, otherNode);
			U64 nodePairKey = Math::I32PairKey(nodesPair);
			U64 nodePairReverseKey = Math::I32PairKey(nodesPairReverse);
			if (m_PrimalityMap[nodePairKey])
			{
				callback(m_BroadToNarrowContactMap[nodePairKey]);
				m_BroadToNarrowContactMap.erase(m_BroadToNarrowContactMap.find(nodePairKey));
			}
			else
			{
				callback(m_BroadToNarrowContactMap[nodePairReverseKey]);
				m_BroadToNarrowContactMap.erase(m_BroadToNarrowContactMap.find(nodePairReverseKey));
			}
			m_ContactsMap[otherNode].erase(std::ranges::find(m_ContactsMap[otherNode], nodeId));
			m_NumberOfContacts--;
			m_PrimalityMap.erase(m_PrimalityMap.find(nodePairKey));
			m_PrimalityMap.erase(m_PrimalityMap.find(nodePairReverseKey));
		}
		m_ContactsMap.erase(m_ContactsMap.find(nodeId));
	}
}
