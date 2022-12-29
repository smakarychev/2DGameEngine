#pragma once

#include "Engine/Core/Camera.h"
#include "Engine/ECS/EntityId.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"
#include <queue>

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
        static Component::Camera& AddDefault2DCamera(Scene& scene, Entity entity, CameraController::ControllerType type = CameraController::ControllerType::Editor2D);

        // Reflects component state to physics state.
        static void SynchronizePhysics(Scene& scene, Entity entity, PhysicsSynchroSetting synchroSetting = PhysicsSynchroSetting::Full);
        static void SynchronizePhysics(Scene& scene);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Scene& scene, Entity entity);
        static void SynchronizeWithPhysicsLocalTransforms(Scene& scene, Entity entity);

        static void AddChild(Scene& scene, Entity parent, Entity child);
        static void RemoveChild(Scene& scene, Entity child);

        template <typename Fn>
        static void TraverseTreeAndApply(Entity startNode, Registry& registry, Fn fn);
        static Entity FindTopOfTree(Entity treeEntity, Registry& registry);
    };

    template <typename Fn>
    void SceneUtils::TraverseTreeAndApply(Entity startNode, Registry& registry, Fn fn)
    {
        std::queue<Entity> entityQueue;
        entityQueue.push(startNode);
        while (!entityQueue.empty())
        {
            Entity curr = entityQueue.front(); entityQueue.pop();

            fn(curr);
            
            if (registry.Has<Component::ChildRel>(curr))
            {
                auto& childRel = registry.Get<Component::ChildRel>(curr);
                Entity child = childRel.First;
                for (U32 childI = 0; childI < childRel.ChildrenCount; childI++)
                {
                    entityQueue.push(child);
                    child = registry.Get<Component::ParentRel>(child).Next;
                }
            }
        }
    }
}


