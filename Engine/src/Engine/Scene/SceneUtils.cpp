#include "enginepch.h"
#include "SceneUtils.h"
#include "Scene.h"
#include "Engine/Core/Core.h"

namespace Engine
{
    void SceneUtils::AddDefaultPhysicalRigidBody2D(Entity& entity, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(entity.HasComponent<Component::Transform2D>(), "Transform is unset")
        ENGINE_CORE_ASSERT(entity.HasComponent<Component::RigidBody2D>(), "RigidBody is unset")
        
        auto& rb = entity.GetComponent<Component::RigidBody2D>();
        Physics::RigidBodyDef2D rbDef;
        rbDef.UserData = static_cast<void*>(&entity);
        rb.PhysicsBody = world2D.CreateBody(rbDef);
    }

    void SceneUtils::AddDefaultBoxCollider2D(Entity& entity, Component::BoxCollider2D& boxCollider2D, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(entity.HasComponent<Component::RigidBody2D>(), "RigidBody is unset")
        auto& rb = entity.GetComponent<Component::RigidBody2D>();
        
        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(boxCollider2D.Offset, boxCollider2D.HalfSize);
        colDef.Collider = &box;
        boxCollider2D.PhysicsCollider = static_cast<Physics::BoxCollider2D*>(world2D.AddCollider(rb.PhysicsBody, colDef));
    }

    void SceneUtils::SynchronizePhysics(Entity& entity, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(entity.HasComponent<Component::Transform2D>(), "Transform is unset")
        
        if (entity.HasComponent<Component::RigidBody2D>() == false)
        {
            ENGINE_CORE_WARN("Rigid body component is unset, while explicitly requesting to sync physics.");
            return;
        }
        auto& tf = entity.GetComponent<Component::Transform2D>();
        auto& rb = entity.GetComponent<Component::RigidBody2D>();
        if (rb.PhysicsBody == nullptr)
        {
            AddDefaultPhysicalRigidBody2D(entity, world2D);
        }
        // Write component state to physics state.
        Physics::RigidBody2D* prb = rb.PhysicsBody;
        prb->SetPosition(tf.Position);
        prb->SetRotation(tf.Rotation);
        prb->SetFlags(rb.Flags);
        const Physics::RigidBodyType2D prevType = prb->GetType();
        prb->SetType(rb.Type);
        
        if (entity.HasComponent<Component::BoxCollider2D>() == false)
        {
            // No warning, since it is valid to have r bodies without colliders.
            return;
        }
        auto& col = entity.GetComponent<Component::BoxCollider2D>();
        if (col.PhysicsCollider == nullptr)
        {
            AddDefaultBoxCollider2D(entity, col, world2D);
        }
        // Write component state to physics state.
        Physics::BoxCollider2D* pcol = col.PhysicsCollider;
        pcol->Center = col.Offset;
        pcol->HalfSize = col.HalfSize;
        pcol->SetSensor(col.IsSensor);
        pcol->SetPhysicsMaterial(col.PhysicsMaterial);
        Component::BoxCollider2D* next = col.Next;
        while (next)
        {
            if (next->PhysicsCollider == nullptr)
            {
                AddDefaultBoxCollider2D(entity, *next, world2D);
                // Write component state to physics state.
                Physics::BoxCollider2D* pcol = next->PhysicsCollider;
                pcol->Center = next->Offset;
                pcol->HalfSize = next->HalfSize;
                pcol->SetSensor(next->IsSensor);
                pcol->SetPhysicsMaterial(next->PhysicsMaterial);
                next = next->Next;
            }
        }

        if (prb->GetType() == Physics::RigidBodyType2D::Dynamic && prevType != prb->GetType()) prb->RecalculateMass();
    }

    void SceneUtils::SynchronizeWithPhysics(Entity& entity, Physics::RigidBodyWorld2D& world2D)
    {
        ENGINE_CORE_ASSERT(entity.HasComponent<Component::Transform2D>(), "Transform is unset")
        if (entity.HasComponent<Component::RigidBody2D>() == false)
        {
            ENGINE_CORE_WARN("Rigid body component is unset, while explicitly requesting to sync with physics.");
            return;
        }
        auto& tf = entity.GetComponent<Component::Transform2D>();
        auto& rb = entity.GetComponent<Component::RigidBody2D>();
        if (rb.PhysicsBody == nullptr)
        {
            ENGINE_CORE_WARN("Entity {} has no physical rigid body attached.", entity.Id);
        }
        tf.Position = rb.PhysicsBody->GetPosition();
        tf.Rotation = rb.PhysicsBody->GetRotation();
    }
}
