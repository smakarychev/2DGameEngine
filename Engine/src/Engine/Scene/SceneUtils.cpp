#include "enginepch.h"
#include "SceneUtils.h"
#include "Scene.h"
#include "Engine/Core/Core.h"
#include "Engine/ECS/View.h"
#include "imgui/imgui.h"

namespace Engine
{
    void SceneUtils::AddDefaultPhysicalRigidBody2D(Scene& scene, Entity entityId)
    {
        auto& registry = scene.GetRegistry();
        auto& world2D = scene.GetRigidBodyWorld2D();
        ENGINE_CORE_ASSERT(registry.Has<Component::Transform2D>(entityId), "Transform is unset")
        ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entityId), "RigidBody is unset")
        
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        Physics::RigidBodyDef2D rbDef;
        rbDef.UserData = reinterpret_cast<void*>(static_cast<U64>(entityId.Id));
        rbDef.Flags = rb.Flags;
        rb.PhysicsBody = RefCountHandle(
            world2D.CreateBody(rbDef),
            [&](void* obj)
            {
                auto* rigidBody = static_cast<Physics::RigidBody2D*>(obj);
                Entity e = static_cast<I32>(reinterpret_cast<U64>(rigidBody->GetUserData()));
                if (registry.Has<Component::BoxCollider2D>(e)) registry.Remove<Component::BoxCollider2D>(e);
                world2D.RemoveBody(static_cast<Physics::RigidBody2D*>(obj));
            }
        );
    }

    void SceneUtils::AddDefaultBoxCollider2D(Scene& scene, Entity entityId, Component::BoxCollider2D& boxCollider2D)
    {
        auto& registry = scene.GetRegistry();
        auto& world2D = scene.GetRigidBodyWorld2D();
        ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entityId), "RigidBody is unset")
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        
        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(boxCollider2D.Offset, boxCollider2D.HalfSize);
        colDef.Collider = &box;
        boxCollider2D.PhysicsCollider = RefCountHandle(
            static_cast<Physics::BoxCollider2D*>(world2D.AddCollider(rb.PhysicsBody.Get(), colDef)),
            [&](void* obj){ world2D.RemoveCollider(rb.PhysicsBody.Get(), (Physics::Collider2D*)obj); }
        );
    }

    void SceneUtils::SynchronizePhysics(Scene& scene, Entity entityId, PhysicsSynchroSetting synchroSetting)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::Transform2D>(entityId), "Transform is unset")
        
        if (registry.Has<Component::RigidBody2D>(entityId) == false)
        {
            ENGINE_CORE_WARN("Rigid body component is unset, while explicitly requesting to sync physics.");
            return;
        }
        auto& tf = registry.Get<Component::Transform2D>(entityId);
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        if (rb.PhysicsBody == nullptr)
        {
            AddDefaultPhysicalRigidBody2D(scene, entityId);
        }
        // Write component state to physics state.
        Physics::RigidBody2D* prb = rb.PhysicsBody.Get();
        Physics::RigidBodyType2D prevType = prb->GetType();
        switch (synchroSetting)
        {
        case PhysicsSynchroSetting::Full:
        {
            prb->SetPosition(tf.Position);
            prb->SetRotation(tf.Rotation);
            prb->SetFlags(rb.Flags);
            prb->SetType(rb.Type);
            if (registry.Has<Component::BoxCollider2D>(entityId) == false)
            {
                // No warning, since it is valid to have r bodies without colliders.
                return;
            }
            auto& col = registry.Get<Component::BoxCollider2D>(entityId);
            if (col.PhysicsCollider.Get() == nullptr)
            {
                AddDefaultBoxCollider2D(scene, entityId, col);
            }
            // Write component state to physics state.
            Physics::BoxCollider2D* pcol = col.PhysicsCollider.Get();
            pcol->Center = col.Offset;
            pcol->HalfSize = col.HalfSize * tf.Scale;
            pcol->SetSensor(col.IsSensor);
            pcol->SetPhysicsMaterial(col.PhysicsMaterial);

            if (prb->GetType() == Physics::RigidBodyType2D::Dynamic && prevType != prb->GetType()) prb->RecalculateMass();
            break;
        }
        case PhysicsSynchroSetting::RBOnly:
        {
            prb->SetPosition(tf.Position);
            prb->SetRotation(tf.Rotation);
            prb->SetFlags(rb.Flags);
            prevType = prb->GetType();
            prb->SetType(rb.Type);
        }
        case PhysicsSynchroSetting::ColliderOnly:
        {
            if (registry.Has<Component::BoxCollider2D>(entityId) == false)
            {
                // No warning, since it is valid to have r bodies without colliders.
                return;
            }
            auto& col = registry.Get<Component::BoxCollider2D>(entityId);
            if (col.PhysicsCollider == nullptr)
            {
                AddDefaultBoxCollider2D(scene, entityId, col);
            }
            // Write component state to physics state.
            Physics::BoxCollider2D* pcol = col.PhysicsCollider.Get();
            pcol->Center = col.Offset;
            pcol->HalfSize = col.HalfSize * tf.Scale;
            pcol->SetSensor(col.IsSensor);
            pcol->SetPhysicsMaterial(col.PhysicsMaterial);
            break;
        }
        }
        if (prb->GetType() == Physics::RigidBodyType2D::Dynamic && prevType != prb->GetType()) prb->RecalculateMass();
    }

    void SceneUtils::SynchronizeWithPhysics(Scene& scene, Entity entityId)
    {
        auto& registry = scene.GetRegistry();
        ENGINE_CORE_ASSERT(registry.Has<Component::Transform2D>(entityId), "Transform is unset")
        if (registry.Has<Component::RigidBody2D>(entityId) == false)
        {
            ENGINE_CORE_WARN("Rigid body component is unset, while explicitly requesting to sync with physics.");
            return;
        }
        auto& tf = registry.Get<Component::Transform2D>(entityId);
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        if (rb.PhysicsBody == nullptr)
        {
            ENGINE_CORE_WARN("Entity {} has no physical rigid body attached.", entityId.GetIndex());
        }
        tf.Position = rb.PhysicsBody->GetPosition();
        tf.Rotation = rb.PhysicsBody->GetRotation();
    }

    void SceneUtils::AddChild(Scene& scene, Entity parent, Entity child)
    {
        auto& registry = scene.GetRegistry();
        auto& childRel = registry.Get<Component::ChildRel>(parent);
        if (childRel.ChildrenCount == 0)
        {
            childRel.First = child;
            auto& parentRel = registry.Add<Component::ParentRel>(child);
            childRel.ChildrenCount = 1;
            parentRel.Parent = parent;
            return;
        }

        childRel.ChildrenCount++;
        auto& newParentRel = registry.Add<Component::ParentRel>(child);
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
    }
}
