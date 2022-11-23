#include "enginepch.h"
#include "SceneUtils.h"
#include "Scene.h"
#include "Engine/Core/Core.h"

namespace Engine
{
    void SceneUtils::AddDefaultPhysicalRigidBody2D(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(registry.Has<Component::Transform2D>(entityId), "Transform is unset")
        ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entityId), "RigidBody is unset")
        
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        Physics::RigidBodyDef2D rbDef;
        rbDef.UserData = (void*)(entityId.Id);
        rb.PhysicsBody = world2D.CreateBody(rbDef);
    }

    void SceneUtils::AddDefaultBoxCollider2D(Registry& registry, Entity entityId, Component::BoxCollider2D& boxCollider2D, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(registry.Has<Component::RigidBody2D>(entityId), "RigidBody is unset")
        auto& rb = registry.Get<Component::RigidBody2D>(entityId);
        
        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(boxCollider2D.Offset, boxCollider2D.HalfSize);
        colDef.Collider = &box;
        boxCollider2D.PhysicsCollider = static_cast<Physics::BoxCollider2D*>(world2D.AddCollider(rb.PhysicsBody, colDef));
    }

    void SceneUtils::SynchronizePhysics(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D)
    {
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
            AddDefaultPhysicalRigidBody2D(registry, entityId, world2D);
        }
        // Write component state to physics state.
        Physics::RigidBody2D* prb = rb.PhysicsBody;
        prb->SetPosition(tf.Position);
        prb->SetRotation(tf.Rotation);
        prb->SetFlags(rb.Flags);
        const Physics::RigidBodyType2D prevType = prb->GetType();
        prb->SetType(rb.Type);
        
        if (registry.Has<Component::BoxCollider2D>(entityId) == false)
        {
            // No warning, since it is valid to have r bodies without colliders.
            return;
        }
        auto& col = registry.Get<Component::BoxCollider2D>(entityId);
        if (col.PhysicsCollider == nullptr)
        {
            AddDefaultBoxCollider2D(registry, entityId, col, world2D);
        }
        // Write component state to physics state.
        Physics::BoxCollider2D* pcol = col.PhysicsCollider;
        pcol->Center = col.Offset;
        pcol->HalfSize = col.HalfSize * tf.Scale;
        pcol->SetSensor(col.IsSensor);
        pcol->SetPhysicsMaterial(col.PhysicsMaterial);
        Component::BoxCollider2D* next = col.Next;
        while (next)
        {
            if (next->PhysicsCollider == nullptr)
            {
                AddDefaultBoxCollider2D(registry, entityId, *next, world2D);
            }
            // Write component state to physics state.
            pcol = next->PhysicsCollider;
            pcol->Center = next->Offset;
            pcol->HalfSize = next->HalfSize * tf.Scale;
            pcol->SetSensor(next->IsSensor);
            pcol->SetPhysicsMaterial(next->PhysicsMaterial);
            next = next->Next;
        }

        if (prb->GetType() == Physics::RigidBodyType2D::Dynamic && prevType != prb->GetType()) prb->RecalculateMass();
    }

    void SceneUtils::SynchronizeWithPhysics(Registry& registry, Entity entityId, Physics::RigidBodyWorld2D& world2D)
    {
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
}
