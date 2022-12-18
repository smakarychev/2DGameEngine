#pragma once

#include "Engine/ECS/EntityId.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace  Engine
{
    class Scene;

    class SceneUtils
    {
    public:
        enum class PhysicsSynchroSetting { Full, RBOnly, ColliderOnly };
    public:
        static void AddDefaultPhysicalRigidBody2D(Scene& scene, Entity entityId);
        static void AddDefaultBoxCollider2D(Scene& scene, Entity entityId, Component::BoxCollider2D& boxCollider2D);

        // Reflects component state to physics state.
        static void SynchronizePhysics(Scene& scene, Entity entityId, PhysicsSynchroSetting synchroSetting = PhysicsSynchroSetting::Full);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Scene& scene, Entity entityId);

        static void AddChild(Scene& scene, Entity parent, Entity child);
        static void RemoveChild(Scene& scene, Entity child);
    };
}


