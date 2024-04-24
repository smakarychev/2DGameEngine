#pragma once

#include "Collision/Colliders/Collider2D.h"
#include "Engine/Math/Hash.h"
#include "Engine/Physics/NewRBE/Newest/RigidBody.h"

namespace Engine::WIP::Physics::Newest
{
    struct BroadContactPair;
    using BodyPairHash = U64;
    struct BodyPair
    {
        RigidBodyId2D First{RB_INVALID_ID};
        RigidBodyId2D Second{RB_INVALID_ID};
        BodyPair(RigidBody2D* first, RigidBody2D* second);
        BodyPair(Collider2D* first, Collider2D* second);
        BodyPair(const BroadContactPair& bcp);
        BodyPairHash GetHash() const;
    };
}