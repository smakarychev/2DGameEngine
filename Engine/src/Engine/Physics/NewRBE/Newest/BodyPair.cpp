#include "enginepch.h"

#include "BodyPair.h"

#include "Collision/BroadPhase/BroadPhase.h"

namespace Engine::WIP::Physics::Newest
{
    BodyPair::BodyPair(RigidBody2D* first, RigidBody2D* second)
    {
        ENGINE_CORE_ASSERT(first->GetId() != second->GetId(), "Body cannot form pair with itself.")
        First = Math::Min(first->GetId(), second->GetId());
        Second = Math::Max(first->GetId(), second->GetId());
    }

    BodyPair::BodyPair(Collider2D* first, Collider2D* second)
    {
        ENGINE_CORE_ASSERT(first != second, "Collider cannot from pair with itself.")
        RigidBody2D* bodyA = first->GetRigidBody();
        RigidBody2D* bodyB = second->GetRigidBody();
        ENGINE_CORE_ASSERT(bodyA->GetId() != bodyB->GetId(), "Body cannot form pair with itself.")
        First = Math::Min(bodyA->GetId(), bodyB->GetId());
        Second = Math::Max(bodyA->GetId(), bodyB->GetId());
    }

    BodyPair::BodyPair(const BroadContactPair& bcp) : BodyPair(bcp.First, bcp.Second)
    {
    }

    BodyPairHash BodyPair::GetHash() const
    {
        return Math::HashBytes(this, sizeof(*this));
    }
}
