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
        struct EntityTransformPair
        {
            Entity Entity;
            Component::LocalToWorldTransform2D& Transform2D;
        };
    public:
        enum class PhysicsSynchroSetting { Full, RBOnly, ColliderOnly };
    public:
        static EntityTransformPair AddDefaultEntity(Scene& scene, const std::string& tag);
        static void AddDefaultPhysicalRigidBody2D(Scene& scene, Entity entity);
        static void AddDefaultBoxCollider2D(Scene& scene, Entity entity);
        static Component::Camera& AddDefault2DCamera(Scene& scene, Entity entity);

        // Reflects component state to physics state.
        static void SynchronizePhysics(Scene& scene, Entity entity, PhysicsSynchroSetting synchroSetting = PhysicsSynchroSetting::Full);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Scene& scene, Entity entity);


        static void AddChild(Scene& scene, Entity parent, Entity child);
        static void RemoveChild(Scene& scene, Entity child);
    };
}


