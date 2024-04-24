#pragma once
#include "Collision/Colliders/Collider2D.h"
#include "Engine/Physics/NewRBE/Newest/RigidBody.h"

namespace Engine::WIP::Physics::Newest
{
    class PhysicsSystem;
    
    enum class StartUpBehaviour { SetActive, SetInactive };

    using ActiveBodyIterator = std::vector<RigidBodyId2D>::const_iterator;
    
    class BodyManager
    {
        struct FreeList
        {
            // Store 1 at lsb to distinguish it from valid pointer.
            FreeList(U32 next) { m_Val = static_cast<U64>(next) << 32 | 1; }
            FreeList(RigidBody2D* body) { m_Val = reinterpret_cast<U64>(body); }
            U32 GetNext() { return static_cast<U32>(m_Val >> 32); }
            operator RigidBody2D*() { return reinterpret_cast<RigidBody2D*>(m_Val); }
        private:
            U64 m_Val{0};
        };
        using FreeListIndex = U32;
    public:
        void Init(PhysicsSystem* physicsSystem, U32 maxBodyCount);
        void ShutDown();
        RigidBody2D* CreateBody(const RigidBodyDesc2D& rbDesc);
        RigidBody2D* AddBody(RigidBody2D* rb, StartUpBehaviour suBehaviour);
        void RemoveBody(RigidBodyId2D rbId);
        void DeleteBody(RigidBodyId2D rbId);

        Collider2D* SetCollider(RigidBodyId2D rbId, const ColliderDesc2D& colDef);

        const std::vector<RigidBody2D*>& GetBodies() const { return m_Bodies; }
        const std::vector<RigidBodyId2D>& GetActiveBodies() const { return m_ActiveBodies; }
        RigidBody2D* GetBody(RigidBodyId2D rbId) const { return m_Bodies[rbId]; }

        U32 GetBodyCount() const { return static_cast<U32>(m_Bodies.size()); }
        U32 GetActiveBodyCount() const { return static_cast<U32>(m_ActiveBodies.size()); }

        bool IsBodyValid(RigidBody2D* rb) const { return !(reinterpret_cast<U64>(rb) & 1); }

        void ActivateBody(RigidBodyId2D rbId);
        void TryActivateBody(RigidBodyId2D rbId);
        void DeactivateBody(RigidBodyId2D rbId);
    private:
        RigidBodyId2D AddOrReuse(RigidBody2D* rb);
        void SwapAndPopFromActive(RigidBody2D* body, U32 activeIndex);
    private:
        PhysicsSystem* m_PhysicsSystem{nullptr};
        
        std::vector<RigidBody2D*> m_Bodies;
        static constexpr auto FL_INVALID_INDEX = std::numeric_limits<U32>::max();
        FreeListIndex m_FirstFreeRb{FL_INVALID_INDEX};
        
        std::vector<RigidBodyId2D> m_ActiveBodies;

        U32 m_MaxBodyCount{0};
        
    };
    
}
