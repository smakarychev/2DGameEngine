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
        
        enum class AssetPayloadType { Prefab, Scene, Image, Font, Unknown };

        enum class LocalTransformPolicy
        {
            // Update local transform, so that object stays at the same place.
            Default,
            // Update local transform to be equal to current world transform (as a result, world transform will change).
            SameAsWorld
        };
        
    public:
        static EntityTransformPair AddDefaultEntity(Scene& scene, const std::string& tag);
        static void DeleteEntity(Scene& scene, Entity entity);
        static void AddDefaultPhysicalRigidBody2D(Scene& scene, Entity entity);
        static void AddDefaultBoxCollider2D(Scene& scene, Entity entity);
        static Component::Camera& AddDefault2DCamera(Scene& scene, Entity entity, CameraController::ControllerType type = CameraController::ControllerType::Editor2D);

        // TODO: this is very questionable function: transforms are attached to rb and colliders via pointers,
        // so when we are deleting entities with rb and/or colliders with swap-and-pop technique, these pointers get
        // messed up. This function reattaches transforms to rb and colliders.
        static void PreparePhysics(Scene& scene);
        // Reflects component state to physics state.
        static void SynchronizePhysics(Scene& scene, Entity entity, PhysicsSynchroSetting synchroSetting = PhysicsSynchroSetting::Full);
        static void SynchronizePhysics(Scene& scene);
        // Reflects physics state to component state.
        static void SynchronizeWithPhysics(Scene& scene, Entity entity);
        static void SynchronizeWithPhysicsLocalTransforms(Scene& scene, Entity entity);

        static void AddChild(Scene& scene, Entity parent, Entity child, LocalTransformPolicy localTransformPolicy = LocalTransformPolicy::Default);
        static void AddChild(Scene& scene, Entity parent, Entity child, bool usePrefabConstraints, LocalTransformPolicy localTransformPolicy = LocalTransformPolicy::Default);
        static void RemoveChild(Scene& scene, Entity child);

        static void EnqueueImmediateChildren(Entity startNode, Registry& registry, std::queue<Entity>& entityQueue);
        template <typename Fn>
        static void TraverseTreeAndApply(Entity startNode, Registry& registry, Fn fn);
        template <typename Fn>
        static void TraverseExceptRootAndApply(Entity startNode, Registry& registry, Fn fn);
        
        static Entity FindTopOfTree(Entity treeEntity, Registry& registry);

        static glm::vec2 GetMousePosition(Scene& scene);
        static bool HasEntityUnderMouse(const glm::vec2& mousePos, FrameBuffer* frameBuffer);
        static Entity GetEntityUnderMouse(const glm::vec2& mousePos, FrameBuffer* frameBuffer);
        
        static AssetPayloadType GetAssetPayloadTypeFromString(const std::string& fileName);
        
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

            EnqueueImmediateChildren(curr, registry, entityQueue);
        }
    }

    template <typename Fn>
    void SceneUtils::TraverseExceptRootAndApply(Entity startNode, Registry& registry, Fn fn)
    {
        std::queue<Entity> entityQueue;

        EnqueueImmediateChildren(startNode, registry, entityQueue);
        
        while (!entityQueue.empty())
        {
            Entity curr = entityQueue.front(); entityQueue.pop();

            fn(curr);
            
            EnqueueImmediateChildren(curr, registry, entityQueue);
        }
    }
}


