#include "enginepch.h"

#include "BroadPhase.h"

#include "Engine/Physics/NewRBE/Newest/PhysicsSystem.h"

namespace Engine::WIP::Physics::Newest
{
    void BroadPhase2D::Init(PhysicsSystem* physicsSystem, Ref<BroadPhaseLayers> bpLayers, Ref<BodyToBroadPhaseLayerFilter> bpFilter)
    {
        m_PhysicsSystem = physicsSystem;
        m_BroadPhaseLayers = bpLayers;
        m_BodyToBroadPhaseLayerFilter = bpFilter;
        m_Trees.resize(bpLayers->GetLayersCount());
    }

    void BroadPhase2D::RegisterBody(RigidBodyId2D rbId)
    {
        const BodyManager& bodyManager = m_PhysicsSystem->GetBodyManager();
        RigidBody2D* body = bodyManager.GetBodies()[rbId];
        ENGINE_CORE_CHECK_RETURN(body->HasCollider(), "Body must have collider to be registered in broad phase.")

        if (body->IsInBroadPhase()) UnregisterBody(rbId);
        
        // Map body to specific tree.
        CollisionLayer broadLayer = body->GetCollisionLayer();
        ENGINE_CORE_ASSERT(broadLayer < m_Trees.size(), "Collision layer is greater than total amount of layers.")
        U32 bpIndex = m_Trees[broadLayer].Insert(body, body->GetBounds());
        body->SetIndexInBroadPhase(bpIndex);
    }

    void BroadPhase2D::UnregisterBody(RigidBodyId2D rbId)
    {
        const BodyManager& bodyManager = m_PhysicsSystem->GetBodyManager();
        RigidBody2D* body = bodyManager.GetBodies()[rbId];
        ENGINE_CORE_CHECK_RETURN(body->IsInBroadPhase(), "Body is not registered in broad phase.")
        // Map body to specific tree.
        CollisionLayer broadLayer = body->GetCollisionLayer();
        m_Trees[broadLayer].Remove(body->GetIndexInBroadPhase());
        body->SetIndexInBroadPhase(RB_INVALID_ID);
    }

    void BroadPhase2D::MoveBody(RigidBodyId2D rbId, const glm::vec2& vel)
    {
        const BodyManager& bodyManager = m_PhysicsSystem->GetBodyManager();
        RigidBody2D* body = bodyManager.GetBodies()[rbId];
        ENGINE_CORE_CHECK_RETURN(body->IsInBroadPhase(), "Body is not registered in broad phase.")
        // Map body to specific tree.
        CollisionLayer broadLayer = body->GetCollisionLayer();
        m_Trees[broadLayer].Move(body->GetIndexInBroadPhase(), body->GetBounds(), vel);
    }

    std::vector<BroadContactPair> BroadPhase2D::GetPairs(ActiveBodyIterator begin, ActiveBodyIterator end) const
    {
        std::vector<BroadContactPair> pairs;
        // TODO: sort bodies by collision layer, and query trees independently.
        const BodyManager& bodyManager = m_PhysicsSystem->GetBodyManager();
        auto& bodies = bodyManager.GetBodies();
        for (auto rbIt = begin; rbIt != end; ++rbIt)
        {
            RigidBody2D* body = bodies[*rbIt];
            if (!body->IsInBroadPhase()) continue;
            // Map body to specific tree.
            for (auto& tree : m_Trees)
            {
                if(!m_BodyToBroadPhaseLayerFilter->ShouldCollide(body->GetCollisionLayer(), tree.GetCollisionLayer())) continue;
                tree.Query(
                    [&](U32 bpNode) { AddPair(pairs, body, reinterpret_cast<RigidBody2D*>(tree.GetPayload(bpNode))); },
                    body->GetBounds());
            }
        }
        return pairs;
    }

    void BroadPhase2D::AddPair(std::vector<BroadContactPair>& pairs, RigidBody2D* first, RigidBody2D* second) const
    {
        if (first == second) return;
        if (second->IsInActiveBodies() && second->GetId() < first->GetId()) return;
        RigidBody2D* primary = first->GetId() < second->GetId() ? first : second;
        RigidBody2D* secondary = primary == first ? second : first;
        pairs.emplace_back(primary->GetCollider(), secondary->GetCollider());
    }
}
