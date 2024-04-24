#pragma once

#include "../../RigidBody.h"
#include "BVHTree.h"
#include "Engine/Physics/NewRBE/Newest/BodyManager.h"

namespace Engine::WIP::Physics::Newest
{
	class PhysicsSystem;

	struct BroadContactPair
	{
		Collider2D* First{nullptr};
		Collider2D* Second{nullptr};
	};
	
	class BroadPhase2D
	{
	public:
		void Init(PhysicsSystem* physicsSystem, Ref<BroadPhaseLayers> bpLayers, Ref<BodyToBroadPhaseLayerFilter> bpFilter);
		void RegisterBody(RigidBodyId2D rbId);
		void UnregisterBody(RigidBodyId2D rbId);
		void MoveBody(RigidBodyId2D rbId, const glm::vec2& vel);
		std::vector<BroadContactPair> GetPairs(ActiveBodyIterator begin, ActiveBodyIterator end) const;
		const std::vector<BVHTree2D>& GetTrees() const { return m_Trees; }
	private:
		void AddPair(std::vector<BroadContactPair>& pairs, RigidBody2D* first, RigidBody2D* second) const;
	private:
		std::vector<BVHTree2D> m_Trees;
		PhysicsSystem* m_PhysicsSystem{nullptr};
		Ref<BroadPhaseLayers> m_BroadPhaseLayers;
		Ref<BodyToBroadPhaseLayerFilter> m_BodyToBroadPhaseLayerFilter;
	};
}
