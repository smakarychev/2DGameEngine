#pragma once

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "BVHTree.h"

#include <array>

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

	// TODO: find a better name?
	template <typename Bounds>
	class BroadPhase
	{
	public:
		void Query(const Bounds& bounds);
	private:
		void QueryCallback(I32 otherNode);
	private:
		BVHTree<Bounds> m_Tree;
		std::vector<PotentialContact> m_Contacts;
		// Node that may form a pair.
		I32 m_CurrentlyTestedNode;
	};

	template<typename Bounds>
	inline void BroadPhase<Bounds>::Query(const Bounds& bounds)
	{
		m_Tree.Query(QueryCallback, bounds);
	}

	template<typename Bounds>
	inline void BroadPhase<Bounds>::QueryCallback(I32 otherNode)
	{
		if (otherNode == m_CurrentlyTestedNode) return;
		// `otherNode` will later become m_CurrentlyTestedNode, so we don't record it twice.
		if (m_Tree.IsMoved(otherNode) && otherNode > m_CurrentlyTestedNode) return;

		m_Contacts.emplace_back(
			reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(m_CurrentlyTestedNode)),
			reinterpret_cast<RigidBody2D*>(m_Tree.GetPayload(otherNode))
		);
	}

}
