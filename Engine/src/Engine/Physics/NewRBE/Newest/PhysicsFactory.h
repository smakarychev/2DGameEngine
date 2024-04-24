#pragma once

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Physics/NewRBE/Newest/RigidBody.h"

namespace Engine::WIP::Physics::Newest
{
    struct ColliderDesc2D;

    // Used to create and destroy physics objects (using it's own set of allocators).
    class PhysicsFactory
    {
        using BodyAllocator = MemoryManager::ManagedPoolAllocator;
        using ColliderAllocator = MemoryManager::ManagedPoolAllocator;
        using DynamicsDataAllocator = MemoryManager::ManagedPoolAllocator;
        
    public:
        static void Init();
        static void ShutDown();
        static PhysicsFactory& Get();

        RigidBody2D* AllocateBody(const RigidBodyDesc2D& rbDesc);
        void DeallocateBody(RigidBody2D* rb);
        Collider2D* AllocateCollider(const ColliderDesc2D& colDesc);
        void DeallocateCollider(Collider2D* col);
        
    private:
        static PhysicsFactory* s_Instance;
        Ref<BodyAllocator> m_BodyAllocator{nullptr};
        Ref<DynamicsDataAllocator> m_DynamicsDataAllocator{nullptr};
        Ref<ColliderAllocator> m_ColliderAllocator{nullptr};
    };
    
    
    
}
