#include "enginepch.h"

#include "Lists.h"

#include "Engine/Physics/NewRBE/RigidBody.h"
#include "Engine/Physics/NewRBE/Collision/BroadPhase.h"
#include "Engine/Physics/NewRBE/Collision/Contacts.h"

namespace Engine::WIP::Physics
{
    RigidBody2D* RigidBodyListEntry2D::CreateElement(MemoryManager::ManagedPoolAllocator& allocator,
        const RigidBodyDef2D& rbDef)
    {
        return NewAlloc<RigidBody2D>(allocator, rbDef);
    }

    void RigidBodyListEntry2D::DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, RigidBody2D* rigidBody)
    {
        return DeleteAlloc(allocator, rigidBody);
    }

    void RigidBodyListEntry2D::SetEntry(RigidBody2D* rigidBody, RigidBodyListEntry2D* entry)
    {
        rigidBody->SetListEntry(entry);
    }

    RigidBodyListEntry2D* RigidBodyListEntry2D::GetEntry(RigidBody2D* rigidBody)
    {
        return rigidBody->GetListEntry();
    }

    U32 RigidBodyListEntry2D::GetElementSize()
    {
        return sizeof(RigidBody2D);
    }

    Collider2D* ColliderListEntry2D::CreateElement(MemoryManager::ManagedPoolAllocator& allocator,
                                                   const ColliderDef2D& colDef)
    {
        return colDef.Clone(allocator);
    }

    void ColliderListEntry2D::DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, Collider2D* collider)
    {
        return Collider2D::Destroy(allocator, collider);
    }

    void ColliderListEntry2D::SetEntry(Collider2D* collider, ColliderListEntry2D* entry)
    {
        collider->SetListEntry(entry);
    }

    ColliderListEntry2D* ColliderListEntry2D::GetEntry(Collider2D* collider)
    {
        return collider->GetListEntry();
    }
    
    U32 ColliderListEntry2D::GetElementSize()
    {
        return Math::Max(sizeof(Collider2D), Math::Max(sizeof(PolygonCollider2D), Math::Max(sizeof(CircleCollider2D), sizeof(EdgeCollider2D))));
    }

    ContactInfo2D* ContactInfoEntry2D::CreateElement(MemoryManager::ManagedPoolAllocator& allocator,
        const PotentialContact2D& potentialContact)
    {
        ContactInfo2D* info = NewAlloc<ContactInfo2D>(allocator);
        info->Colliders = potentialContact.Colliders;
        info->Manifold = nullptr;
        info->AccumulatedNormalImpulses = info->AccumulatedTangentImpulses = {0.0f, 0.0f};
        // Check if any of colliders is sensor, and set flag if so.
        info->SetSensors();
        return info;
    }

    void ContactInfoEntry2D::DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, ContactInfo2D* contactInfo)
    {
        if (contactInfo->Manifold) Delete(contactInfo->Manifold);
        DeleteAlloc(allocator, contactInfo);
    }

    void ContactInfoEntry2D::SetEntry(ContactInfo2D* contactInfo, ContactInfoEntry2D* entry)
    {
        contactInfo->ContactInfoEntry = entry;
    }

    ContactInfoEntry2D* ContactInfoEntry2D::GetEntry(ContactInfo2D* contactInfo)
    {
        return contactInfo->ContactInfoEntry;
    }
    
    U32 ContactInfoEntry2D::GetElementSize()
    {
        return sizeof(ContactInfo2D);
    }
}


