#include "enginepch.h"

#include "PhysicsFactory.h"

#include "Collision/Colliders/Collider2D.h"
#include "Collision/Colliders/CircleCollider2D.h"
#include "Collision/Colliders/CompoundCollider2D.h"
#include "Collision/Colliders/EdgeCollider2D.h"
#include "Collision/Colliders/PolygonCollider2D.h"

namespace Engine::WIP::Physics::Newest
{
    PhysicsFactory* PhysicsFactory::s_Instance = nullptr;

    void PhysicsFactory::Init()
    {
        ENGINE_CORE_ASSERT(s_Instance == nullptr, "PhysicsFactory is already initialized.")
        s_Instance = New<PhysicsFactory>();
        
        // TODO: init with max number of bodies?
        s_Instance->m_BodyAllocator = CreateRef<BodyAllocator>(sizeof(RigidBody2D));
        s_Instance->m_DynamicsDataAllocator = CreateRef<DynamicsDataAllocator>(sizeof(DynamicsData2D));
        U64 colliderTypeSize = Math::Max(sizeof(Collider2D),
            Math::Max(sizeof(PolygonCollider2D),
                Math::Max(sizeof(CircleCollider2D),
                    Math::Max(sizeof(CompoundCollider2D), sizeof(EdgeCollider2D)))));
        s_Instance->m_ColliderAllocator = CreateRef<ColliderAllocator>(colliderTypeSize);
    }

    void PhysicsFactory::ShutDown()
    {
        s_Instance->m_ColliderAllocator.reset();
        s_Instance->m_DynamicsDataAllocator.reset();
        s_Instance->m_BodyAllocator.reset();
        Delete(s_Instance);
        s_Instance = nullptr;
    }

    PhysicsFactory& PhysicsFactory::Get()
    {
        ENGINE_CORE_ASSERT(s_Instance != nullptr, "PhysicsFactory is uninitiallized.")
        return *s_Instance;
    }

    RigidBody2D* PhysicsFactory::AllocateBody(const RigidBodyDesc2D& rbDesc)
    {
        RigidBody2D* newBody = NewAlloc<RigidBody2D>(*m_BodyAllocator, rbDesc);
        if (rbDesc.BodyType != BodyType::Static)
        {
            DynamicsData2D* newDD = NewAlloc<DynamicsData2D>(*m_DynamicsDataAllocator, rbDesc.DynamicsDataDesc);
            newBody->SetDynamicsData(newDD);
        }
        return newBody;
    }

    void PhysicsFactory::DeallocateBody(RigidBody2D* rb)
    {
        if (rb->GetType() != BodyType::Static)
        {
            DynamicsData2D* bodyDD = rb->m_DynamicsData;
            DeleteAlloc(*m_DynamicsDataAllocator, bodyDD);
        }
        DeleteAlloc(*m_BodyAllocator, rb);
    }

    Collider2D* PhysicsFactory::AllocateCollider(const ColliderDesc2D& colDesc)
    {
        switch (colDesc.m_ColliderType)
        {
            case Collider2DType::Polygon:   return NewAlloc<PolygonCollider2D>(*m_ColliderAllocator, static_cast<const PolygonColliderDesc2D&>(colDesc));
            case Collider2DType::Circle:    return NewAlloc<CircleCollider2D>(*m_ColliderAllocator, static_cast<const CircleColliderDesc2D&>(colDesc));
            case Collider2DType::Edge:      return NewAlloc<EdgeCollider2D>(*m_ColliderAllocator, static_cast<const EdgeColliderDesc2D&>(colDesc));
            case Collider2DType::Compound:  return NewAlloc<CompoundCollider2D>(*m_ColliderAllocator, static_cast<const CompoundColliderDesc2D&>(colDesc));
            case Collider2DType::TypesCount:
                ENGINE_CORE_ASSERT(false, "Unknown collider type.") return nullptr;
        }
        return nullptr;
    }

    void PhysicsFactory::DeallocateCollider(Collider2D* col)
    {
        switch (col->GetType())
        {
            case Collider2DType::Polygon:   DeleteAlloc(*m_ColliderAllocator, (PolygonCollider2D*)col); break;
            case Collider2DType::Circle:    DeleteAlloc(*m_ColliderAllocator, (CircleCollider2D*)col); break;
            case Collider2DType::Edge:      DeleteAlloc(*m_ColliderAllocator, (EdgeCollider2D*)col); break;
            case Collider2DType::Compound:
                for (auto* subCol: ((CompoundCollider2D*)col)->m_SubColliders) DeallocateCollider(subCol);
                DeleteAlloc(*m_ColliderAllocator, (CompoundCollider2D*)col); break;
            case Collider2DType::TypesCount: ENGINE_CORE_ASSERT(false, "Unknown collider type.") break;
        }
    }
}
