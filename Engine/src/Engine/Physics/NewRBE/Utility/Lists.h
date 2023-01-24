#pragma once
#include "Engine/Memory/MemoryManager.h"
#include "Allocators.h"

namespace Engine::WIP::Physics
{
    struct PotentialContact2D;
    struct ContactInfo2D;
    struct ColliderDef2D;
    class Collider2D;
    struct RigidBodyDef2D;
    class RigidBody2D;

    template <typename T>
    struct PhysicsListsIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        PhysicsListsIterator(T* element) : m_Element(element) {}
            
        reference operator*() const { return *m_Element; }
        pointer operator->() { return m_Element; }
        PhysicsListsIterator operator++() { m_Element = reinterpret_cast<T*>(m_Element->Next); return *this; }
        friend bool operator==(const PhysicsListsIterator& a, const PhysicsListsIterator& b) { return a.m_Element == b.m_Element; }
        friend bool operator!=(const PhysicsListsIterator& a, const PhysicsListsIterator& b) { return !(a == b); }
    private:
        T* m_Element{nullptr};
    };

    template <typename Impl, typename T>
    struct GeneralListEntryBase
    {
        using ElementType = T;
        
        T* Element{nullptr};
        GeneralListEntryBase<Impl, T>* Next{nullptr};
        GeneralListEntryBase<Impl, T>* Prev{nullptr};
        template <typename Alloc, typename ... Args>
        static T* CreateElement(Alloc& alloc, Args&& ... args)
        {
            return Impl::CreateElement(alloc, std::forward<Args>(args)...);
        }
        template <typename Alloc>
        static void DestroyElement(Alloc& alloc, T* element)
        {
            return Impl::DestroyElement(alloc, element);
        }
        static void SetEntry(T* element, GeneralListEntryBase* entry)
        {
            return Impl::SetEntry(element, reinterpret_cast<Impl*>(entry));
        }
        static GeneralListEntryBase* GetEntry(T* element)
        {
            return reinterpret_cast<GeneralListEntryBase*>(Impl::GetEntry(element));
        }
        static U32 GetElementSize()
        {
            return Impl::GetElementSize();
        }
    };

    template <typename Entry, typename Allocators>
    class GeneralList
    {
    public:
        GeneralList() = default;
        template <typename ... Args>
        typename Entry::ElementType* Push(Args&& ... args);
        void Pop(typename Entry::ElementType* element);
        void Clear();
        U32 Size() { return m_ElementsCount; }
        
        PhysicsListsIterator<Entry> begin() { return PhysicsListsIterator(m_ListHead); }
        PhysicsListsIterator<Entry> end() { return PhysicsListsIterator<Entry>(nullptr); }

        PhysicsListsIterator<Entry> begin() const { return PhysicsListsIterator(m_ListHead); }
        PhysicsListsIterator<Entry> end() const { return PhysicsListsIterator<Entry>(nullptr); }
    private:
        Entry* m_ListHead{nullptr};
        U32 m_ElementsCount{0};
    };

    template <typename Entry, typename Allocators>
    template <typename ... Args>
    typename Entry::ElementType* GeneralList<Entry, Allocators>::Push(Args&& ... args)
    {
        m_ElementsCount++;
        typename Entry::ElementType* newElement = Entry::CreateElement(*Allocators::s_ElementAllocator, std::forward<Args>(args)...);
        Entry* newEntry = NewAlloc<Entry>(*Allocators::s_ListEntryAllocator);
        newEntry->Element = newElement;
        Entry::SetEntry(newElement, newEntry);
        newEntry->Next = m_ListHead;
        if (m_ListHead != nullptr) m_ListHead->Prev = newEntry;
        m_ListHead = newEntry;
        return newElement;
    }

    template <typename Entry, typename Allocators>
    void GeneralList<Entry, Allocators>::Pop(typename Entry::ElementType* element)
    {
        m_ElementsCount--;
        Entry* entry = Entry::GetEntry(element);
        if (entry == m_ListHead) m_ListHead = reinterpret_cast<Entry*>(entry->Next);
        if (entry->Prev) entry->Prev->Next = entry->Next;
        if (entry->Next) entry->Next->Prev = entry->Prev;
        Entry::DestroyElement(*Allocators::s_ElementAllocator, element);
        DeleteAlloc<Entry>(*Allocators::s_ListEntryAllocator, entry);
    }

    template <typename Entry, typename Allocators>
    void GeneralList<Entry, Allocators>::Clear()
    {
        while(m_ListHead)
        {
            Entry* toDel = m_ListHead;
            m_ListHead = reinterpret_cast<Entry*>(m_ListHead->Next);
            Entry::DestroyElement(*Allocators::s_ElementAllocator, toDel->Element);
            DeleteAlloc<Entry>(*Allocators::s_ListEntryAllocator, toDel);
        }
    }

    struct RigidBodyListEntry2D : GeneralListEntryBase<RigidBodyListEntry2D, RigidBody2D>
    {
        RigidBody2D* GetRigidBody() { return Element; }
        static RigidBody2D* CreateElement(MemoryManager::ManagedPoolAllocator& allocator, const RigidBodyDef2D& rbDef);
        static void DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, RigidBody2D* rigidBody);
        static void SetEntry(RigidBody2D* rigidBody, RigidBodyListEntry2D* entry);
        static RigidBodyListEntry2D* GetEntry(RigidBody2D* rigidBody);
        static U32 GetElementSize();
    };

    struct ColliderListEntry2D : GeneralListEntryBase<ColliderListEntry2D, Collider2D>
    {
        Collider2D* GetCollider() { return Element; }
        static Collider2D* CreateElement(MemoryManager::ManagedPoolAllocator& allocator, const ColliderDef2D& colDef);
        static void DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, Collider2D* collider);
        static void SetEntry(Collider2D* collider, ColliderListEntry2D* entry);
        static ColliderListEntry2D* GetEntry(Collider2D* collider);
        static U32 GetElementSize();
    };

    struct ContactInfoEntry2D : GeneralListEntryBase<ContactInfoEntry2D, ContactInfo2D>
    {
        ContactInfo2D* GetContactInfo() { return Element; }
        static ContactInfo2D* CreateElement(MemoryManager::ManagedPoolAllocator& allocator, const PotentialContact2D& potentialContact);
        static void DestroyElement(MemoryManager::ManagedPoolAllocator& allocator, ContactInfo2D* contactInfo);
        static void SetEntry(ContactInfo2D* contactInfo, ContactInfoEntry2D* entry);
        static ContactInfoEntry2D* GetEntry(ContactInfo2D* contactInfo);
        static U32 GetElementSize();
    };

    using BodyListAllocators2D = GeneralListAllocators2D<RigidBodyListEntry2D>;
    using ColliderListAllocators2D = GeneralListAllocators2D<ColliderListEntry2D>;
    using ContactListAllocators2D = GeneralListAllocators2D<ContactInfoEntry2D>;
    
    using RigidBodyList2D = GeneralList<RigidBodyListEntry2D, BodyListAllocators2D>;
    using ColliderList2D = GeneralList<ColliderListEntry2D, ColliderListAllocators2D>;
    using ContactInfoList2D = GeneralList<ContactInfoEntry2D, ContactListAllocators2D>;
    
}
