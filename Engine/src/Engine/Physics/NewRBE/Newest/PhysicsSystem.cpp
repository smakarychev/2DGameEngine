#include "enginepch.h"
#include "PhysicsSystem.h"

#include <utility>

#include "Collision/NarrowPhase/Contact.h"
#include "Collision/NarrowPhase/ContactManager.h"

namespace Engine::WIP::Physics::Newest
{
    void PhysicsSystem::Init(U32 maxBodies, Ref<BroadPhaseLayers> bpLayers, Ref<BodyToBroadPhaseLayerFilter> bpFilter)
    {
        m_BodyManager.Init(this, maxBodies);
        m_BroadPhase.Init(this, std::move(bpLayers), std::move(bpFilter));
        m_IslandManager.Init(maxBodies);
    }

    void PhysicsSystem::ShutDown()
    {
        m_BodyManager.ShutDown();
    }

    void PhysicsSystem::Update(F32 dt)
    {
        UpdateContext(dt);
        
        SynchronizeBroadPhase();
        IntegrateVelocities();

        ProcessCollisions();
        
        //TODO: Resolve velocities.
        IntegratePositions();
        //TODO: Resolve positions.
        
        //TODO: Move it away.
        const std::vector<RigidBody2D*>& bodies = m_BodyManager.GetBodies();
        const std::vector<RigidBodyId2D>& activeBodies = m_BodyManager.GetActiveBodies();
        for (RigidBodyId2D id : activeBodies)
        {
            RigidBody2D* body = bodies[id];
            body->RecalculateBounds();
        }
        
        //TODO: Put bodies to sleep.
    }

    void PhysicsSystem::UpdateContext(F32 dt)
    {
        m_FrameContext.DeltaTime = dt;
        m_FrameContext.ContactsCache.Swap();
        m_FrameContext.ContactAllocator.Clear();
    }

    void PhysicsSystem::UpdateBodyCollider(RigidBodyId2D bodyId)
    {
        const std::vector<RigidBody2D*>& bodies = m_BodyManager.GetBodies();
        RigidBody2D* body = bodies[bodyId];
        ENGINE_CORE_CHECK_RETURN(body->IsInBroadPhase(), "Body is not in the broad phase.")
        if (!body->IsStatic()) body->RecalculateMass();
        body->SetBounds(body->GetCollider()->GenerateBounds(body->GetTransform()));
        m_BroadPhase.MoveBody(bodyId, body->GetLinearVelocity() * m_FrameContext.DeltaTime);
    }

    void PhysicsSystem::IntegrateVelocities()
    {
        F32 dt = m_FrameContext.DeltaTime;
        const std::vector<RigidBody2D*>& bodies = m_BodyManager.GetBodies();
        const std::vector<RigidBodyId2D>& activeBodies = m_BodyManager.GetActiveBodies();
        for (RigidBodyId2D id : activeBodies)
        {
            RigidBody2D* body = bodies[id];
            DynamicsData2D& dd = body->GetDynamicsData();
            glm::vec2 linAcc = { 0.0f, 0.0f };
            F32 angAcc = 0.0f;
            // If body has finite mass, convert its force to acceleration.
            if (dd.HasFiniteMass())
            {
                linAcc = dd.GetGravityMultiplier() * m_Settings.GravityVector;
                linAcc += dd.GetForce() * dd.GetInverseMass();
            }
            // Same logic for inertia tensor and angular acceleration.
            if (dd.HasFiniteInertia())
            {
                angAcc += dd.GetTorque() * dd.GetInverseInertia();
            }

            glm::vec2 newLinVel = dd.GetLinearVelocity() + linAcc * dt;
            F32 newAngVel = dd.GetAngularVelocity() + angAcc * dt;
            // Apply damping.
            newLinVel = newLinVel / (1.0f + dt * dd.GetLinearDamping());
            newAngVel = newAngVel / (1.0f + dt * dd.GetAngularDamping());
            // Update body vel.
            dd.SetLinearVelocity(newLinVel);
            dd.SetAngularVelocity(newAngVel);
        }
    }

    void PhysicsSystem::ProcessCollisions()
    {
        m_IslandManager.Clear();
        U32 processedBodiesCount = 0;
        while(m_BodyManager.GetActiveBodyCount() > processedBodiesCount)
        {
            auto pairs = m_BroadPhase.GetPairs(
                m_BodyManager.GetActiveBodies().begin() + processedBodiesCount,
                m_BodyManager.GetActiveBodies().end());

            processedBodiesCount = m_BodyManager.GetActiveBodyCount();
            
            ProcessPairs(pairs);
        }
        m_IslandManager.Finalize(m_BodyManager.GetActiveBodies());
    }

    void PhysicsSystem::ProcessPairs(const std::vector<BroadContactPair>& pairs)
    {
        // First try to find this pair in cache.
        // If it is in cache, it means that there was a contact frame before,
        // and we can either call `OnContactPersists` or `OnContactEnd`
        // TODO: try to use manifold from previous frame, if bodies didn't move far enough.

        auto& readB = m_FrameContext.ContactsCache.GetReadBuffer();
        auto& writeB = m_FrameContext.ContactsCache.GetWriteBuffer();
                
        for (const auto& pair : pairs)
        {
            BodyPair bp{pair};
            BodyPairHash bpHash = bp.GetHash();
            bool hadContact = readB.contains(bpHash);

            bool canCollide = CollisionFilter::ShouldCollide(pair.First, pair.Second);
            if (!canCollide && hadContact) { /* TODO: call OnContactEnd */; continue; }

            Contact2D* contact = ContactManager::Create(m_FrameContext.ContactAllocator, pair.First, pair.Second);
            ContactInfo2D contactInfo{};
            bool hasContact = contact->GenerateContacts(contactInfo) > 0;
            if (hasContact)
            {
                // Link bodies to island.
                if (!hadContact)
                {
                    /* TODO: call OnContactBegin */;
                    m_BodyManager.TryActivateBody(bp.First);
                    m_BodyManager.TryActivateBody(bp.Second);
                    m_IslandManager.LinkBodies(
                        m_BodyManager.GetBody(bp.First)->GetIndexInActiveBodies(),
                        m_BodyManager.GetBody(bp.Second)->GetIndexInActiveBodies()
                    );
                }
            }
            else
            {
                if (hadContact) /* TODO: call OnContactEnd */;
            }

            writeB.emplace(bpHash, contactInfo);
        }
    }

    void PhysicsSystem::IntegratePositions()
    {
        F32 dt = m_FrameContext.DeltaTime;
        const std::vector<RigidBody2D*>& bodies = m_BodyManager.GetBodies();
        const std::vector<RigidBodyId2D>& activeBodies = m_BodyManager.GetActiveBodies();
        for (RigidBodyId2D id : activeBodies)
        {
            RigidBody2D* body = bodies[id];
            DynamicsData2D& dd = body->GetDynamicsData();
            glm::vec2 newPos = body->GetPosition() + dd.GetLinearVelocity() * dt;
            F32 deltaRot = dd.GetAngularVelocity() * dt;
            body->SetPosition(newPos);
            body->AddRotation(deltaRot);
            dd.ResetForce();
            dd.ResetTorque();
        }
    }

    void PhysicsSystem::SynchronizeBroadPhase()
    {
        F32 dt = m_FrameContext.DeltaTime;
        const std::vector<RigidBody2D*>& bodies = m_BodyManager.GetBodies();
        const std::vector<RigidBodyId2D>& activeBodies = m_BodyManager.GetActiveBodies();
        for (RigidBodyId2D id : activeBodies)
        {
            RigidBody2D* body = bodies[id];
            if (body->IsInBroadPhase()) m_BroadPhase.MoveBody(id, body->GetLinearVelocityU() * dt);
        }
    }
}
