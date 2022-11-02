#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace  Engine
{
    class SceneUtils
    {
    public:
        static void AddDefaultPhysicalRigidBody2D(Entity& entity, Physics::RigidBodyWorld2D& world2D);
        static void AddDefaultBoxCollider2D(Entity& entity, Component::BoxCollider2D& boxCollider2D, Physics::RigidBodyWorld2D& world2D);

        // Reflects component state to physics state.
        static void SynchronizePhysics(Entity& entity, Physics::RigidBodyWorld2D& world2D);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Entity& entity, Physics::RigidBodyWorld2D& world2D);

    };
}


