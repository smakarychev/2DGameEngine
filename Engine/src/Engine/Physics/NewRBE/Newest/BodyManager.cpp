#include "enginepch.h"

#include "BodyManager.h"

#include "PhysicsFactory.h"
#include "PhysicsSystem.h"

namespace Engine::WIP::Physics::Newest
{
    void BodyManager::Init(PhysicsSystem* physicsSystem, U32 maxBodyCount)
    {
        m_PhysicsSystem = physicsSystem;
        m_MaxBodyCount = maxBodyCount;
        m_Bodies.reserve(maxBodyCount);
        m_ActiveBodies.reserve(maxBodyCount);
    }

    void BodyManager::ShutDown()
    {
        for (auto* rb : m_Bodies)
        {
            if (IsBodyValid(rb)) DeleteBody(rb->GetId());
        }
    }

    RigidBody2D* BodyManager::CreateBody(const RigidBodyDesc2D& rbDesc)
    {
        RigidBody2D* newBody = PhysicsFactory::Get().AllocateBody(rbDesc);
        return newBody;
    }

    RigidBody2D* BodyManager::AddBody(RigidBody2D* rb, StartUpBehaviour suBehaviour)
    {
        // Check that we have enough memory space.
        ENGINE_CORE_CHECK_RETURN_NULL(m_Bodies.size() < m_MaxBodyCount, "BodyManager: error on adding body. Max count is reached.")
        // Check that body was not already added.
        ENGINE_CORE_CHECK_RETURN_NULL(rb->GetId() == RB_INVALID_ID, "BodyManager: trying to add body that already exists") 

        RigidBodyId2D bodyId = AddOrReuse(rb);
        rb->m_Id = bodyId;

        if (suBehaviour == StartUpBehaviour::SetInactive || rb->IsStatic()) return rb;

        rb->SetIndexInActiveBodies(static_cast<U32>(m_ActiveBodies.size()));
        m_ActiveBodies.push_back(bodyId);
        
        return rb;
    }

    void BodyManager::RemoveBody(RigidBodyId2D rbId)
    {
        ENGINE_CORE_CHECK_RETURN(rbId != RB_INVALID_ID, "BodyManager: trying to remove non-existing  body.")
        RigidBody2D* body = m_Bodies[rbId];
        if (body->IsInActiveBodies())
        {
            DeactivateBody(rbId);
        }
        if (body->IsInBroadPhase())
        {
            m_PhysicsSystem->GetBroadPhase().UnregisterBody(rbId);
        }
    }

    void BodyManager::DeleteBody(RigidBodyId2D rbId)
    {
        ENGINE_CORE_CHECK_RETURN(rbId != RB_INVALID_ID, "BodyManager: trying to delete non-existing  body.")
        RigidBody2D* body = m_Bodies[rbId];
        m_Bodies[rbId] = FreeList(m_FirstFreeRb);
        m_FirstFreeRb = rbId;
        if (body->GetCollider() != nullptr) PhysicsFactory::Get().DeallocateCollider(body->GetCollider());
        PhysicsFactory::Get().DeallocateBody(body);
    }

    Collider2D* BodyManager::SetCollider(RigidBodyId2D rbId, const ColliderDesc2D& colDef)
    {
        ENGINE_CORE_ASSERT(rbId != RB_INVALID_ID, "Body does not exist.")
        RigidBody2D* body = m_Bodies[rbId];
        ENGINE_CORE_CHECK_RETURN_NULL(body->GetCollider() == nullptr, "Body already has an allocator.")
        Collider2D* newCol = PhysicsFactory::Get().AllocateCollider(colDef);
        body->SetCollider(newCol);
        newCol->SetRigidBody(body);
        // Add to broad phase.
        m_PhysicsSystem->GetBroadPhase().RegisterBody(rbId);
        return newCol;
    }

    void BodyManager::ActivateBody(RigidBodyId2D rbId)
    {
        RigidBody2D* body = m_Bodies[rbId];
        ENGINE_CHECK_RETURN(!body->IsInActiveBodies(), "Body is already active.")
        body->SetIndexInActiveBodies(static_cast<RigidBodyId2D>(m_ActiveBodies.size()));
        m_ActiveBodies.push_back(rbId);
    }

    void BodyManager::TryActivateBody(RigidBodyId2D rbId)
    {
        RigidBody2D* body = m_Bodies[rbId];
        if (body->IsInActiveBodies() || !body->IsDynamic()) return;
        body->SetIndexInActiveBodies(static_cast<RigidBodyId2D>(m_ActiveBodies.size()));
        m_ActiveBodies.push_back(rbId);
    }

    void BodyManager::DeactivateBody(RigidBodyId2D rbId)
    {
        RigidBody2D* body = m_Bodies[rbId];
        U32 activeIndex = body->GetIndexInActiveBodiesU();
        SwapAndPopFromActive(body, activeIndex);
    }

    RigidBodyId2D BodyManager::AddOrReuse(RigidBody2D* rb)
    {
        // Check if we have a free element (a hole in vector).
        if (m_FirstFreeRb != FL_INVALID_INDEX)
        {
            // Reuse it.
            FreeListIndex freeIndex = m_FirstFreeRb;
            m_FirstFreeRb = FreeList(m_Bodies[freeIndex]).GetNext();
            m_Bodies[freeIndex] = rb;
            return freeIndex;
        }
        // Add new body.
        m_Bodies.push_back(rb);
        return static_cast<U32>(m_Bodies.size()) - 1;
    }

    void BodyManager::SwapAndPopFromActive(RigidBody2D* body, U32 activeIndex)
    {
        if (m_ActiveBodies.size() > 1)
        {
            U32 lastIndex = static_cast<U32>(m_ActiveBodies.size()) - 1;
            RigidBody2D* lastBody = m_Bodies[m_ActiveBodies.back()];
            std::swap(body->m_DynamicsData->m_IndexInActiveBodies, lastBody->m_DynamicsData->m_IndexInActiveBodies);
            std::swap(m_ActiveBodies[activeIndex], m_ActiveBodies[lastIndex]);
        }
        m_ActiveBodies.pop_back();
        body->SetIndexInActiveBodies(RB_INVALID_ID);
    }
}
