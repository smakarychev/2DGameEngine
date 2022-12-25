#include "enginepch.h"
#include "SceneUtils.h"
#include "Scene.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Core.h"
#include "Engine/ECS/View.h"
#include "imgui/imgui.h"

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

    void SceneUtils::AddDefaultBoxCollider2D(Scene& scene, Entity entity, Component::BoxCollider2D& boxCollider2D)
    {
        auto& registry = scene.GetRegistry();
        auto& world2D = scene.GetRigidBodyWorld2D();
        
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

    void SceneUtils::AddDefault2DCamera(Scene& scene, Entity entity)
    {
        auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
        glm::vec2 viewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
        camera->SetViewport((U32)viewportSize.x, (U32)viewportSize.y);
        camera->SetProjection(Camera::ProjectionType::Orthographic);
        camera->SetZoom(6.0f);
        auto cameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);

        FrameBuffer::Spec spec;
        spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
        spec.Attachments = {
            { FrameBuffer::Spec::AttachmentFormat::Color,			      FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::RedInteger,          FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::Depth24Stencil8,     FrameBuffer::Spec::AttachmentCategory::Write },
        };
        auto frameBuffer = FrameBuffer::Create(spec);

        auto& registry = scene.GetRegistry();
        auto& tf = registry.Get<Component::LocalToWorldTransform2D>(entity);
        auto& cameraComp = registry.Add<Component::Camera>(entity);
        cameraComp.CameraController = cameraController;
        cameraComp.CameraFrameBuffer = frameBuffer;
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
            if (col.PhysicsCollider.Get() == nullptr) AddDefaultBoxCollider2D(scene, entity, col);
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



    void SceneUtils::AddChild(Scene& scene, Entity parent, Entity child)
    {
        auto& registry = scene.GetRegistry();
        if (!registry.Has<Component::ChildRel>(parent)) registry.Add<Component::ChildRel>(parent);
        auto& childRel = registry.Get<Component::ChildRel>(parent);
        if (childRel.ChildrenCount == 0)
        {
            childRel.First = child;
            auto& parentRel = registry.Add<Component::ParentRel>(child);
            registry.Add<Component::LocalToParentTransform2D>(child) = registry.Get<Component::LocalToWorldTransform2D>(child);
            if (registry.Has<Component::ParentRel>(parent))
            {
                parentRel.Depth = registry.Get<Component::ParentRel>(parent).Depth + 1;
            }
            childRel.ChildrenCount = 1;
            parentRel.Parent = parent;
            return;
        }

        childRel.ChildrenCount++;
        auto& newParentRel = registry.Add<Component::ParentRel>(child);
        registry.Add<Component::LocalToParentTransform2D>(child) = registry.Get<Component::LocalToWorldTransform2D>(child);
        newParentRel.Parent = parent;

        if (childRel.ChildrenCount != 1)
        {
            auto& firstChildParentRel = registry.Get<Component::ParentRel>(childRel.First);
            firstChildParentRel.Prev = child;
            newParentRel.Next = childRel.First;
        }
        
        childRel.First = child;
    }

    void SceneUtils::RemoveChild(Scene& scene, Entity child)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::ParentRel>(child), "Entity has no parent.")
        auto& parentRel = registry.Get<Component::ParentRel>(child);

        Entity parent = parentRel.Parent;
        auto& parentChildRef = registry.Get<Component::ChildRel>(parent);

        if (parentRel.Prev != NULL_ENTITY) registry.Get<Component::ParentRel>(parentRel.Prev).Next = parentRel.Next;
        if (parentRel.Next != NULL_ENTITY) registry.Get<Component::ParentRel>(parentRel.Next).Prev = parentRel.Prev;
        if (child == parentChildRef.First) parentChildRef.First = parentRel.Next;
        parentChildRef.ChildrenCount--;
        registry.Remove<Component::ParentRel>(child);
        registry.Remove<Component::LocalToParentTransform2D>(child);
        if (parentChildRef.ChildrenCount == 0) registry.Remove<Component::ChildRel>(parent);
    }
}
