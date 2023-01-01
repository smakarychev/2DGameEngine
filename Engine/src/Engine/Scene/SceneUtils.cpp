#include "enginepch.h"
#include "SceneUtils.h"
#include "Scene.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Core.h"
#include "Engine/ECS/View.h"
#include "imgui/imgui.h"
#include "Serialization/Prefab.h"

namespace
{
    using namespace Engine;
    bool BFS(Entity startNode, Entity toFindNode, Registry& registry)
    {
        std::queue<Entity> entityQueue;
        entityQueue.push(startNode);
        while (!entityQueue.empty())
        {
            Entity curr = entityQueue.front(); entityQueue.pop();
            if (curr == toFindNode) return true;
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
        return false;
    }
}

namespace Engine
{
    SceneUtils::EntityTransformPair SceneUtils::AddDefaultEntity(Scene& scene, const std::string& tag)
    {
        auto& registry = scene.GetRegistry();
        Entity entity = registry.CreateEntity(tag);
        // We need both of these, because first is convenient for physics,
        // and second for rendering and scene hierarchies.
        auto& tf = registry.Add<Component::LocalToWorldTransform2D>(entity);
        return {entity, tf};
    }

    void SceneUtils::DeleteEntity(Scene& scene, Entity entity)
    {
        // Before deleting entity, detach it from parent (if any) and delete it's children (if any).
        auto& registry = scene.GetRegistry();
        if (registry.Has<Component::ChildRel>(entity))
        {
            auto& childRel = registry.Get<Component::ChildRel>(entity);
            Entity child = childRel.First;
            for (U32 childI = 0; childI < childRel.ChildrenCount; childI++)
            {
                Entity next = registry.Get<Component::ParentRel>(child).Next;
                DeleteEntity(scene, child);
                child = next;
            }
        }
        if (registry.Has<Component::ParentRel>(entity)) RemoveChild(scene, entity);
        registry.DeleteEntity(entity);
    }

    void SceneUtils::AddDefaultPhysicalRigidBody2D(Scene& scene, Entity entity)
    {
        auto& registry = scene.GetRegistry();
        auto& world2D = scene.GetRigidBodyWorld2D();
        ENGINE_CORE_ASSERT(registry.Has<Component::LocalToWorldTransform2D>(entity), "Transform is unset")
        ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entity), "RigidBody is unset")
        
        auto& rb = registry.Get<Component::RigidBody2D>(entity);
        Physics::RigidBodyDef2D rbDef;
        rbDef.UserData = reinterpret_cast<void*>(static_cast<U64>(entity.Id));
        rbDef.Flags = rb.Flags;
        rbDef.AttachedTransform = &registry.Get<Component::LocalToWorldTransform2D>(entity);
        rb.PhysicsBody = RefCountHandle(
            world2D.CreateBody(rbDef),
            [&](void* obj)
            {
                world2D.RemoveBody(static_cast<Physics::RigidBody2D*>(obj));
            }
        );
    }

    void SceneUtils::AddDefaultBoxCollider2D(Scene& scene, Entity entity)
    {
        auto& registry = scene.GetRegistry();
        auto& world2D = scene.GetRigidBodyWorld2D();
        auto& boxCollider2D = registry.Get<Component::BoxCollider2D>(entity);
        
        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(boxCollider2D.Offset, boxCollider2D.HalfSize);
        colDef.Collider = &box;
        colDef.UserData = reinterpret_cast<void*>(static_cast<U64>(entity.Id));
        colDef.AttachedTransform = &registry.Get<Component::LocalToWorldTransform2D>(entity);
        if (registry.Has<Component::RigidBody2D>(entity))
        {
            auto& rb = registry.Get<Component::RigidBody2D>(entity);
            boxCollider2D.PhysicsCollider = RefCountHandle(
                static_cast<Physics::BoxCollider2D*>(world2D.SetCollider(rb.PhysicsBody.Get(), colDef)),
                [&](void* obj){ world2D.DeleteCollider((Physics::Collider2D*)obj); }
            );
        }
        else
        {
            boxCollider2D.PhysicsCollider = RefCountHandle(
                static_cast<Physics::BoxCollider2D*>(world2D.AddCollider(colDef)),
                [&](void* obj){ world2D.DeleteCollider((Physics::Collider2D*)obj); }
            );
        }
        
    }

    Component::Camera& SceneUtils::AddDefault2DCamera(Scene& scene, Entity entity, CameraController::ControllerType type)
    {
        auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
        glm::vec2 viewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
        camera->SetViewport((U32)viewportSize.x, (U32)viewportSize.y);
        camera->SetProjection(Camera::ProjectionType::Orthographic);
        camera->SetZoom(6.0f);
        auto cameraController = CameraController::Create(type, camera);

        FrameBuffer::Spec spec;
        spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
        spec.Attachments = {
            { FrameBuffer::Spec::AttachmentFormat::Color,			      FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::RedInteger,          FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::Depth24Stencil8,     FrameBuffer::Spec::AttachmentCategory::Write },
        };
        auto frameBuffer = FrameBuffer::Create(spec);

        auto& registry = scene.GetRegistry();
        auto& cameraComp = registry.AddOrGet<Component::Camera>(entity);
        cameraComp.CameraController = cameraController;
        cameraComp.CameraFrameBuffer = frameBuffer;
        return cameraComp;
    }

    void SceneUtils::PreparePhysics(Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        for (auto e : View<Component::BoxCollider2D>(registry))
        {
            auto& tf = registry.Get<Component::LocalToWorldTransform2D>(e);
            auto& col = registry.Get<Component::BoxCollider2D>(e);
            col.PhysicsCollider->SetAttachedTransform(&tf);
            SceneUtils::SynchronizePhysics(scene, e, SceneUtils::PhysicsSynchroSetting::ColliderOnly);
        }
        for (auto e : View<Component::RigidBody2D>(registry))
        {
            auto& tf = registry.Get<Component::LocalToWorldTransform2D>(e);
            auto& rb = registry.Get<Component::RigidBody2D>(e);
            rb.PhysicsBody->SetAttachedTransform(&tf);
        }
    }

    void SceneUtils::SynchronizePhysics(Scene& scene, Entity entity, PhysicsSynchroSetting synchroSetting)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::LocalToWorldTransform2D>(entity), "Transform is unset")
        
        auto& tf = registry.Get<Component::LocalToWorldTransform2D>(entity);

        Physics::RigidBodyType2D prevType = Physics::RigidBodyType2D::None;
        
        if(synchroSetting == PhysicsSynchroSetting::Full || synchroSetting == PhysicsSynchroSetting::RBOnly)
        {
            ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entity), "Entity has no rigid body.")
            auto& rb = registry.Get<Component::RigidBody2D>(entity);
            
            if (rb.PhysicsBody == nullptr) AddDefaultPhysicalRigidBody2D(scene, entity);
            // Write component state to physics state.
            Physics::RigidBody2D* prb = rb.PhysicsBody.Get();
            prevType = prb->GetType();
            prb->SetFlags(rb.Flags);
            prb->SetType(rb.Type);
        }
        if(synchroSetting == PhysicsSynchroSetting::Full || synchroSetting == PhysicsSynchroSetting::ColliderOnly)
        {
            ENGINE_CORE_ASSERT(registry.Has<Component::BoxCollider2D>(entity), "Entity has no box collider.")
            auto& col = registry.Get<Component::BoxCollider2D>(entity);
            if (col.PhysicsCollider.Get() == nullptr) AddDefaultBoxCollider2D(scene, entity);
            // Write component state to physics state.
            Physics::BoxCollider2D* pcol = col.PhysicsCollider.Get();
            pcol->Center = col.Offset;
            pcol->HalfSize = col.HalfSize * tf.Scale;
            pcol->SetSensor(col.IsSensor);
            pcol->SetPhysicsMaterial(col.PhysicsMaterial);
        }
        if (prevType != Physics::RigidBodyType2D::None)
        {
            auto& rb = registry.Get<Component::RigidBody2D>(entity);
            Physics::RigidBody2D* prb = rb.PhysicsBody.Get();
            prb->RecalculateMass();
        }
    }

    void SceneUtils::SynchronizePhysics(Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        for (auto e : View<Component::BoxCollider2D>(registry))
        {
            SceneUtils::SynchronizePhysics(scene, e, SceneUtils::PhysicsSynchroSetting::ColliderOnly);
        }
        for (auto e : View<Component::RigidBody2D>(registry))
        {
            SceneUtils::SynchronizePhysics(scene, e, SceneUtils::PhysicsSynchroSetting::RBOnly);
        }
    }

    void SceneUtils::SynchronizeWithPhysics(Scene& scene, Entity entity)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::LocalToWorldTransform2D>(entity), "Transform is unset")
        if (registry.Has<Component::RigidBody2D>(entity) == false)
        {
            ENGINE_CORE_WARN("Rigid body component is unset, while explicitly requesting to sync with physics.");
            return;
        }
        auto& tf = registry.Get<Component::LocalToWorldTransform2D>(entity);
        auto& localToWorld = registry.Get<Component::LocalToWorldTransform2D>(entity);
        auto& rb = registry.Get<Component::RigidBody2D>(entity);
        if (rb.PhysicsBody == nullptr)
        {
            ENGINE_CORE_WARN("Entity {} has no physical rigid body attached.", entity.GetIndex());
        }
        tf.Position = rb.PhysicsBody->GetPosition();
        tf.Rotation = rb.PhysicsBody->GetRotation();
        localToWorld = {tf.Position, tf.Scale, tf.Rotation};
    }

    void SceneUtils::SynchronizeWithPhysicsLocalTransforms(Scene& scene, Entity entity)
    {
        auto& registry = scene.GetRegistry();
        auto& tf = registry.Get<Component::LocalToWorldTransform2D>(entity);
        Entity parent = registry.Get<Component::ParentRel>(entity).Parent;
        auto& parentTf = registry.Get<Component::LocalToWorldTransform2D>(parent);
        registry.Get<Component::LocalToParentTransform2D>(entity) = tf.Concatenate(parentTf.Inverse());
    }


    void SceneUtils::AddChild(Scene& scene, Entity parent, Entity child, LocalTransformPolicy localTransformPolicy)
    {
        AddChild(scene, parent, child, true, localTransformPolicy);
    }

    void SceneUtils::AddChild(Scene& scene, Entity parent, Entity child, bool usePrefabConstraints, LocalTransformPolicy localTransformPolicy)
    {
        auto& registry = scene.GetRegistry();
        // Check that neither parent nor child belong to prefab - prefabs shall stay untouched.
        if (usePrefabConstraints)
        {
            if (registry.Has<Component::BelongsToPrefab>(parent) ||
                registry.Has<Component::Prefab>(parent) ||
                registry.Has<Component::BelongsToPrefab>(child)) return;
        }
        // Check that child is not parent's parent.
        if (BFS(child, parent, registry)) return;
        // Remove child from previous parent.
        if (registry.Has<Component::ParentRel>(child)) RemoveChild(scene, child);
        
        if (!registry.Has<Component::ChildRel>(parent)) registry.Add<Component::ChildRel>(parent);
        auto& childRel = registry.Get<Component::ChildRel>(parent);

        auto& childParentRel = registry.Add<Component::ParentRel>(child);
        childParentRel.Parent = parent;
        childParentRel.Depth = registry.Has<Component::ParentRel>(parent) ?
            registry.Get<Component::ParentRel>(parent).Depth + 1 :
            1;
        childRel.ChildrenCount++;
        if (childRel.ChildrenCount != 1)
        {
            // Add to "linked list".
            auto& firstChildParentRel = registry.Get<Component::ParentRel>(childRel.First);
            firstChildParentRel.Prev = child;
            childParentRel.Next = childRel.First;
        }
        childRel.First = child;
        // Change transform, so that child stays in place when attached.
        switch (localTransformPolicy)
        {
            case LocalTransformPolicy::Default:
            {
                auto& parentTf = registry.Get<Component::LocalToWorldTransform2D>(parent);
                registry.Add<Component::LocalToParentTransform2D>(child) = registry.Get<Component::LocalToWorldTransform2D>(child).Concatenate(parentTf.Inverse());
                break;
            }
            case LocalTransformPolicy::SameAsWorld:
            {
                registry.Add<Component::LocalToParentTransform2D>(child) = registry.Get<Component::LocalToWorldTransform2D>(child);
                registry.Get<Component::LocalToWorldTransform2D>(child) = Component::LocalToWorldTransform2D{};
                break;
            }
        }
        // Update all `child` children depth.
        TraverseTreeAndApply(child, registry, [&](Entity e)
        {
            if (e == child) return;
            auto& parentRel = registry.Get<Component::ParentRel>(e);
            parentRel.Depth += 1;
        });
    }

    void SceneUtils::RemoveChild(Scene& scene, Entity child)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::ParentRel>(child), "Entity has no parent.")
        // Check that child doesn't belong to prefab - prefabs shall stay untouched.
        if (registry.Has<Component::BelongsToPrefab>(child)) return;
        
        auto& parentRel = registry.Get<Component::ParentRel>(child);

        Entity parent = parentRel.Parent;
        auto& parentChildRef = registry.Get<Component::ChildRel>(parent);

        if (parentRel.Prev != NULL_ENTITY) registry.Get<Component::ParentRel>(parentRel.Prev).Next = parentRel.Next;
        if (parentRel.Next != NULL_ENTITY) registry.Get<Component::ParentRel>(parentRel.Next).Prev = parentRel.Prev;
        if (child == parentChildRef.First) parentChildRef.First = parentRel.Next;
        parentChildRef.ChildrenCount--;

        // Update all `child` children depth.
        TraverseTreeAndApply(child, registry, [&](Entity e)
        {
            if (e == child) return;
            auto& parentRel = registry.Get<Component::ParentRel>(e);
            parentRel.Depth -= 1;
        });

        registry.Remove<Component::ParentRel>(child);
        registry.Remove<Component::LocalToParentTransform2D>(child);
        if (parentChildRef.ChildrenCount == 0) registry.Remove<Component::ChildRel>(parent);
    }

    Entity SceneUtils::FindTopOfTree(Entity treeEntity, Registry& registry)
    {
        Entity curr = treeEntity;
        while (registry.Has<Component::ParentRel>(curr)) curr = registry.Get<Component::ParentRel>(curr).Parent;
        return curr;
    }

    bool SceneUtils::HasEntityUnderMouse(const glm::vec2& mousePos, FrameBuffer* frameBuffer)
    {
        return !(mousePos.x < 0.0f || mousePos.x > ImguiState::MainViewportSize.x ||
            mousePos.y < 0.0f || mousePos.y > ImguiState::MainViewportSize.y);
    }

    Entity SceneUtils::GetEntityUnderMouse(const glm::vec2& mousePos, FrameBuffer* frameBuffer)
    {
        // Read id texture, and get entityId from it.
        I32 entityId = frameBuffer->ReadPixel(1, static_cast<U32>(mousePos.x), static_cast<U32>(mousePos.y),
                                              RendererAPI::DataType::Int);
        return entityId;
    }

    SceneUtils::AssetPayloadType SceneUtils::GetAssetPayloadTypeFromString(const std::string& fileName)
    {
        static std::vector<std::string> sceneExtensions  = { "scene" };
        static std::vector<std::string> prefabExtensions = {"prefab"};
        static std::vector<std::string> imageExtensions  = { "png", "jpg", "jpeg" };
        // TODO: Font.
        
        std::string fileExtension{};
        auto pos = fileName.find_last_of(".");
        if (pos == std::string::npos)
        {
            ENGINE_CORE_ERROR("Invalid file name: {}", fileName);
            return AssetPayloadType::Unknown;
        }
        fileExtension = fileName.substr(pos + 1);
        if (std::ranges::find(sceneExtensions, fileExtension) != sceneExtensions.end()) return AssetPayloadType::Scene;
        if (std::ranges::find(prefabExtensions, fileExtension) != prefabExtensions.end()) return AssetPayloadType::Prefab;
        if (std::ranges::find(imageExtensions, fileExtension) != imageExtensions.end()) return AssetPayloadType::Image;
    }
}
