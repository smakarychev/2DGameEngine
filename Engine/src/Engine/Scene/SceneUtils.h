#pragma once

#include "Engine/ECS/EntityId.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace  Engine
{
    class SceneUtils
    {
    public:
        static void AddDefaultPhysicalRigidBody2D(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D);
        static void AddDefaultBoxCollider2D(Registry& registry, Entity entityId, Component::BoxCollider2D& boxCollider2D, Physics::RigidBodyWorld2D& world2D);

        // Reflects component state to physics state.
        static void SynchronizePhysics(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D);

    };
}


